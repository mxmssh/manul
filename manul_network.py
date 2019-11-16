#   Manul - network file
#   -------------------------------------
#   Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>
#
#   Copyright 2019 Salesforce.com, inc. All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#     http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.


import os
import sys
import socket
import sys
from manul_utils import *
from printing import *
import time
import pickle
import socket

'''
Protocol format:
size nthreads|files_list [content]

'''
MAX_RESPONSE_SIZE = 1500
REQUEST_THREADS_COUNT = "nthreads"
FILES_LIST_SEQUENCE = "files_list"
REQUEST_BITMAP = "bitmap_request"
SEND_BITMAP = "bitmap_send"
REMOTE_BITMAPS_SYNC_FREQUENCY = 15

# TODO: a lot of code duplicates, remove it + make it as class?
def get_slaves_ips(file_path):
    '''
    :param file_path: path to file with list of string in IP:PORT\n format
    :return: list of (ip:port) in necessary form
    '''
    if not os.path.isfile(file_path):
        ERROR("Unable to find file specified %s" % file_path)

    res = list()
    content = open(file_path).readlines()
    for line in content:
        line = line.replace("\n", "")
        if line == "":
            continue
        if line is None or ":" not in line:
            ERROR("Slave's IP:PORT address is in invalid format %s" % line)

        line = line.split(":")
        ip = line[0]
        port = line[1]
        res.append((ip, int(port)))
    return res

def get_remote_threads_count(ips):  # TODO: test it on 3 and more slaves

    res = list()
    total_threads_count = 0

    for ip, port in ips:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            INFO(0, None, None, "Requesting available parallel fuzzers count from %s:%d" % (ip, port))
            sock.connect((ip, port))
            message = "%d %s" % (len(REQUEST_THREADS_COUNT), REQUEST_THREADS_COUNT)
            INFO(1, None, None, "Sending %s" % message)
            sock.sendall(message)
            data = sock.recv(MAX_RESPONSE_SIZE)  # TODO: wait time ?
        except socket.error as exc:
            sock.close()
            ERROR("Request failed, socket return error %s" % exc)

        INFO(1, None, None, "Receiving %s" % data)
        if data is None or len(data) == 0:
            ERROR("Violation of protocol. Slave returned empty string")

        try:
            tokens = data.split(" ")  # 0 - size, 1 - nthreads 2 - threads_count
            token = tokens[1]
            if token != "nthreads":
                sock.close()
                ERROR("Violation of protocol. Received wrong data from slave. Terminating")
            threads_count = int(tokens[2])
        except:
            ERROR("Violation of protocol. Unable to parse %s" % data)

        INFO(0, None, None, "Slave has %d available fuzzers" % threads_count)
        total_threads_count += threads_count

        res.append((ip, port, int(threads_count)))
        sock.close()

    INFO(0, None, None, "Total available fuzzers %d" % total_threads_count)
    return res, total_threads_count

def send_files_list(ip, port, files):
    INFO(0, None, None, "Sending list of %d files to %s:%d" % (len(files), ip, port))
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ip, int(port)))
    except socket.error as exc:
        sock.close()
        ERROR("Failed to connect with slave, socket returned error %s" % exc)

    message = "%s %s" % (FILES_LIST_SEQUENCE, str(files))
    message = str(len(message)) + " " + message
    INFO(1, None, None, "Sending %s" % message)
    try:
        sock.sendall(message)
    except socket.error as exc:
        sock.close()
        ERROR("Failed to send files list to slave, socket returned error %s" % exc)

    sock.close()

def get_files_list_from_master(ip_port, nthreads):
    ip = ip_port.split(":")[0]
    port = int(ip_port.split(":")[1])

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    INFO(0, None, None, "Waiting for incoming job from master")
    try:
        server_address = (ip, port)
        sock.bind(server_address)
        sock.listen(1)
        connection, client_address = sock.accept()
    except socket.error as exc:
        sock.close()
        ERROR("Failed to connect with master, socket returned error %s" % exc)

    INFO(0, None, None, "Incoming connection from %s:%d" % (client_address[0], client_address[1]))

    # Step 1. Master is asking for threads count
    try:
        data = connection.recv(MAX_RESPONSE_SIZE)
    except socket.error as exc:
        sock.close()
        ERROR("Failed to read request, socket returned error %d: %s" % exc)

    INFO(1, None, None, "Received from master: %s" % data)

    if data is None or len(data) == 0:
        ERROR("Protocol violation. Nothing received from master")

    data = data.split(" ")

    if int(data[0]) > MAX_RESPONSE_SIZE:
        sock.close()
        ERROR("Protocol violation. Invalid size. %s")
    elif data[1] != REQUEST_THREADS_COUNT:
        sock.close()
        ERROR("Protocol violation. Invalid token.")

    # Step 2. Answering master with threads count
    message = "%s %d" % (REQUEST_THREADS_COUNT, nthreads)
    message = str(len(message)) + " " + message
    INFO(1, None, None, "Answering master with %s" % message)

    try:
        connection.sendall(message)
    except (socket.error, exc):
        sock.close()
        ERROR("Failed to send threads count to master. Socket return error %s" % exc)

    # Step 3. Master is answering with files list
    INFO(0, None, None, "Waiting for files list")
    try:
        connection, client_address = sock.accept()
    except (socket.error, exc):
        sock.close()
        ERROR("Failed to connect with master %d: %s" % exc)

    files_list = ""
    bytes_to_read = MAX_RESPONSE_SIZE
    bytes_read = 0
    while bytes_read < bytes_to_read:
        try:
            data = connection.recv(bytes_to_read)
        except (socket.error, exc):
            connection.close()
            sock.close()
            ERROR("Failed to read master's response. Socket return error %s" % exc)

        if data is None or len(data) == 0:
            ERROR("Nothing received from master")

        content = data
        INFO(1, None, None, "Received files list from master: %s" % content)
        if bytes_read == 0:  # at first iteration reading size of data that master wants to actually send
            try:
                content = content.split(" ")
                size_needed = int(content[0])
                bytes_to_read = size_needed
                content = content[2:]
                content = "".join(content)
            except:
                connection.close()
                sock.close()
                ERROR("Protocol violation. Failed to parse list of files received %s" % data)

        files_list += content
        bytes_read += len(data)

    # Step 4. Parsing str as list to actual list. Master sends it as list of lists split by thread. I don't want to
    # parse list of lists, so I just split them again later.
    if len(files_list) == 0:
        ERROR("File lists is empty, nothing to fuzz")

    files_list = files_list.replace("[", "")
    files_list = files_list.replace("]", "")
    files_list = files_list.replace("\'", "")
    files_list = files_list.split(",")

    INFO(1, None, None, "Files list from master: %s" % files_list)
    connection.close()
    sock.close()
    return files_list

def sync_bitmap_net(virgin_bits, remote_virgin_bits):
    INFO(0, None, None, "Synchronizing bitmaps")
    new_cov = False
    for i in range(0, SHM_SIZE):
      byte = int(remote_virgin_bits[i]) # TODO: make sure it works
      if byte != 0xFF and virgin_bits[i] == 0xFF:
        virgin_bits[i] = byte
        new_cov = 1
    if new_cov:
        INFO(1, None, None, "New coverage found")
    else:
        INFO(1, None, None, "Nothing new found")

def socket_recv(socket_instance, error_on_empty):
    try:
        rec_data = socket_instance.recv(1500)
    except socket.error as exc:
        socket_instance.close()
        ERROR("Failed to recv data from socket. Socket return error %s" % exc)

    if error_on_empty and (rec_data is None or len(rec_data) == 0):
        socket_instance.close()
        ERROR("Failed to recv data from socket. Data received is 0 length")

    return rec_data

def recv_data(socket_instance):
    data = ""
    # read size of data we are going to recv
    rec_data = socket_recv(socket_instance, True)
    try:
        rec_data = rec_data.split(" ")
        length = int(rec_data[0])
        data = rec_data[1]
    except:
        socket_instance.close()
        ERROR("Protocol violation, length or token is not specified")
    bytes_read = len(data)

    while bytes_read < length:
        data += socket_recv(socket_instance, False)
        bytes_read = len(data)

    INFO(1, None, None, "Raw bytes received %d" % len(data))
    return pickle.loads(data)

def send_data(msg, sock, connection):
    data_str = pickle.dumps(msg)
    package = str(len(data_str)) + " " + data_str
    try:
        connection.send(package)
    except socket.error as exc:
        connection.close()
        sock.close()
        ERROR("Failed to send, socket returned error %d: %s" % exc)
        return 0
    return 1

def receive_bitmap_slave(ip_port, virgin_bits):
    ip = ip_port.split(":")[0]
    port = int(ip_port.split(":")[1])

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (ip, port)
    sock.bind(server_address)
    sock.listen(1)

    while True:
        INFO(0, None, None, "Waiting for incoming job from master on %s:%d" % (ip, port))
        try:
            connection, client_address = sock.accept()
        except socket.error as exc:
            sock.close()
            ERROR("Failed to connect with master, socket returned error %s" % exc)

        INFO(0, None, None, "Incoming connection from %s:%d" % (client_address[0], client_address[1]))

        # Step 1. Master is sending control request
        data = connection.recv(1500)
        data = pickle.loads(data)
        try:
            data = data.split(" ")[1]
        except:
            connection.close()
            socket.close()
            ERROR("Protocol violation invalid token received %s" % data)

        if data.startswith(REQUEST_BITMAP):
            INFO(1, None, None, "Sending bitmap")
            bitmap_to_send = list("\x00" * SHM_SIZE)
            for i, y in enumerate(virgin_bits):
                bitmap_to_send[i] = y
            send_data(bitmap_to_send, sock, connection)
        elif data.startswith(SEND_BITMAP):
            INFO(1, None, None, "Receiving new bitmap")
            data = recv_data(connection)
            INFO(1, None, None, "Synchronizing bitmaps")
            sync_bitmap_net(virgin_bits, data)
        else:
            connection.close()
            sock.close()
            ERROR("Protocol violation. Invalid request from master %" % data)
        connection.close()

    sock.close() # TODO: do we really need to open and close socket each time ?

def sync_remote_bitmaps(virgin_bits, ips):
    '''

    :param virgin_bits: coverage bitmap automatically updated by local fuzzers
    :param is_master: master or slave instance ?
    :return:
    '''
    while True:
        time.sleep(REMOTE_BITMAPS_SYNC_FREQUENCY)
        INFO(1, None, None, "Syncronizing bitmaps")

        # Step 1. Pulling bitmap from slaves
        for ip, port in ips: # TODO: what if our target is running in simple mode ?
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                INFO(1, None, None, "Requesting bitmap from %s:%d" % (ip, port))
                sock.connect((ip, port))
                time.sleep(1)
                message = "%d %s" % (len(REQUEST_BITMAP), REQUEST_BITMAP)
                INFO(1, None, None, "Sending %s" % message)
                data_str = pickle.dumps(message)
                res = sock.sendall(data_str)
                data = recv_data(sock)
            except socket.error as exc:
                sock.close()
                WARNING(None, "Request to %s failed, socket return error %s" % (ip, exc))
                continue

            INFO(1, None, None, "Received %d bitmap" % len(data))
            if data is None or len(data) == 0:
                sock.close()
                ERROR("Violation of protocol. Slave %s returned empty string" % ip)

            #for symbol in data:
            if len(data) != SHM_SIZE:
                sock.close()
                ERROR("Bitmap is less than %d bytes" % SHM_SIZE)

            # TODO: use has_new_bits or sync_bitmap
            sync_bitmap_net(virgin_bits, data)
            sock.close()

        # Step 2. Sending updated bitmap to slaves
        for ip, port in ips: # TODO: what if our target is running in simple mode ?
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                INFO(1, None, None, "Sending bitmap to %s:%d" % (ip, port))
                sock.connect((ip, port))
                message = "%d %s" % (len(SEND_BITMAP), SEND_BITMAP)
                INFO(1, None, None, "Sending %s" % message)
                data_str = pickle.dumps(message)
                sock.sendall(data_str)

                bitmap_to_send = list("\x00" * SHM_SIZE)
                for i, y in enumerate(virgin_bits):
                    bitmap_to_send[i] = y
                raw_data = pickle.dumps(bitmap_to_send)
                msg = str(len(raw_data)) + " " + raw_data
                INFO(1, None, None, "Sending actual bitmap %d" % len(msg))
                sock.sendall(msg)

            except socket.error as exc:
                WARNING(None, "Request to %s failed, socket return error %s" % (ip, exc))
            sock.close()

class Network(object):
    def __init__(self, target_ip, target_port, target_protocol):
        self.target_ip = target_ip
        self.target_port = target_port
        self.target_protocol = target_protocol
        self.s = None
        # open socket
        if self.target_protocol == "tcp":
            self.protocol_l4 = socket.SOCK_STREAM
        else:
            self.protocol_l4 = socket.SOCK_DGRAM

    def send_test_case(self, data):
        self.s = socket.socket(socket.AF_INET, self.protocol_l4)
        INFO(1, None, None, "Connecting to %s on port %d" % (self.target_ip, self.target_port))
        try:
            self.s.connect((self.target_ip, self.target_port))
        except:
            ERROR("Failed to connect to the host specified %s %d" % (self.target_ip, self.target_port))
        INFO(1, None, None, "Sending %d bytes, content %s" % (len(data), data))

        res = self.s.sendall(data)
        if res:
            WARNING(None, "Failed to send data to the server")
        else:
            INFO(1, None, None, "Done")
            # receiving data from the server if any
            #while 1:
            #    data = self.s.recv(4096)
            #    if not data: break
            #    INFO(1, None, None, "Received %d bytes from the server in response", len(data))
        self.s.close()
