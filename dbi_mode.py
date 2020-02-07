from printing import *
import os
import random
import socket
if sys.platform == "win32":
    import win32pipe, win32file, pywintypes, winerror, win32event
from select import select

COMMAND_CYCLE_FINISH = 'K'
COMMAND_CYCLE_START = 'P'
COMMAND_CRASH = 'C'
COMMAND_FINISH = 'F'

def gen_socket_name_lin():
    return "/usr/tmp/manul_uds_socket_%d"

def gen_pipe_name_win():
    return r'\\.\pipe\\manul_dbi_pipe_in_%d' % random.randint(1, 999999999)

def gen_ipc_obj_name():
    if sys.platform == "win32":
        return gen_pipe_name_win()
    elif "linux" in sys.platform:
        return gen_socket_name_lin()
    else:
        ERROR("Invalid platform for DBI persistent mode")

#TODO (high priority): timeouts for pipes on Windows (blocking situations)
class IPCObjectHandler(object):
    def __init__(self, timeout):
        self.ipc_obj_name = gen_ipc_obj_name()
        if sys.platform == "win32":
            self.overlap_read = pywintypes.OVERLAPPED()
            self.pipe_in = None
            self.overlap_read.hEvent = None
        else:
            self.sock = None
            self.conn = None
        self.is_connected = False
        self.timeout = timeout

    def get_ipc_obj_name(self):
        return self.ipc_obj_name

    def setup_pipe_win(self):
        INFO(1, None, None, "Opening PIPE %s" % self.ipc_obj_name)
        self.overlap_read.hEvent = win32event.CreateEvent(None, 0, 0, None)
        if self.overlap_read.hEvent == 0:
            ERROR("Failed to initialize named pipe")
        try:
            self.pipe_in = win32pipe.CreateNamedPipe(self.ipc_obj_name,
                                                     win32pipe.PIPE_ACCESS_DUPLEX | win32file.FILE_FLAG_OVERLAPPED,
                                                     0,
                                                     1, 512, 512,
                                                     0,
                                                     None)
        except OSError as e:
            ERROR("Failed to create named pipe %s! Error %s" % (self.ipc_obj_name, e.message))

    def setup_socket_lin(self):
        # Make sure the socket does not already exist
        try:
            os.unlink(self.ipc_obj_name)
        except OSError:
            if os.path.exists(self.ipc_obj_name):
                ERROR("Socket already exists %s" % self.ipc_obj_name)

        # Create a UDS socket
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        # Bind the socket to the port
        INFO(1, None, None, "Starting UDS on %s" % self.ipc_obj_name)
        self.sock.bind(self.ipc_obj_name)

        # Listen for incoming connections
        self.sock.listen(1)
        self.sock.settimeout(self.timeout)
        self.is_connected = False

    def setup_ipc_object(self):
        if sys.platform == "win32":
            return self.setup_pipe_win()
        elif "linux" in sys.platform:
            return self.setup_socket_lin()
        else:
            ERROR("Invalid platform for DBI persistent mode")

    def close_socket_lin(self):
        if self.sock:
            self.sock.close()
            os.unlink(self.ipc_obj_name)

    def close_pipe_win(self):
        if self.pipe_in:
            win32pipe.DisconnectNamedPipe(self.pipe_in)
            win32file.CloseHandle(self.pipe_in)
            win32file.CloseHandle(self.overlap_read.hEvent)


    def close_ipc_object(self):
        if sys.platform == "win32":
            return self.close_pipe_win()
        elif "linux" in sys.platform:
            return self.close_socket_lin()
        else:
            ERROR("Invalid platform for DBI persistent mode")

    def connect_pipe_win(self):
        if self.is_connected:
            self.is_connected = True
            return
        INFO(1, None, None, "Waiting for client to connect on pipe %s" % self.ipc_obj_name)
        res = win32pipe.ConnectNamedPipe(self.pipe_in, self.overlap_read)
        if res == winerror.ERROR_IO_PENDING:
            INFO(1, None, None, "Client is not ready, waiting")
            win32event.WaitForSingleObject(self.overlap_read.hEvent, win32event.INFINITE)

        INFO(1, None, None, "Client successfully connected to %s, res = %s" % (self.ipc_obj_name, res))

    @staticmethod
    def send_command_win(self, command_str):
        INFO(1, None, None, "Sending %s" % command_str)
        res = win32file.WriteFile(self.pipe_in, command_str.encode("utf-8"), self.overlap_read)
        INFO(1, None, None, "Done sending command over PIPE %s" % self.pipe_in)
        return res

    @staticmethod
    def send_command_lin(self, command_str):
        INFO(1, None, None, "Sending %s into socket %s" % (command_str, self.ipc_obj_name))
        if not self.conn:
            ERROR("Wrong recv/send command order, connection is not yet established!")
        self.conn.send(command_str.encode("utf-8"))
        INFO(1, None, None, "Done sending command in UDS %s" % self.ipc_obj_name)

    def send_command(self, command_str):
        if sys.platform == "win32":
            return self.send_command_win(self, command_str)
        elif "linux" in sys.platform:
            return self.send_command_lin(self, command_str)
        else:
            ERROR("Invalid platform for DBI persistent mode")

    @staticmethod
    def recv_command_win(self):
        INFO(1, None, None, "Reading PIPE %s"  % self.ipc_obj_name)
        #using winAFL implementation for ReadFile with timeout
        res, res_str = win32file.ReadFile(self.pipe_in, 1, self.overlap_read)
        if res == winerror.ERROR_IO_PENDING:
            rc = win32event.WaitForSingleObject(self.overlap_read.hEvent, self.timeout*100)
            if rc != win32event.WAIT_OBJECT_0:
                # winAFL: took longer than specified timeout or other error - cancel read
                win32file.CancelIo(self.pipe_in);
                # wait for cancelation to finish properly.
                win32event.WaitForSingleObject(self.overlap_read.hEvent, win32event.INFINITE);
                return "T"
        res_str = bytes(res_str)
        INFO(1, None, None, "Received %s, result code: %d" % (res_str, res))
        try: # TODO: it would be probably better to use OVERLAPPED API functions here
            res_str = res_str.decode("utf-8")
        except:
            # it means that the target failed to answer us for some reason
            res_str = ""
        return res_str

    @staticmethod
    def recv_command_lin(self):
        INFO(1, None, None, "Reading from UDS %s"  % self.ipc_obj_name)
        res = None

        if not self.is_connected:
            # Wait for a connection
            try:
                self.conn, client_address = self.sock.accept()
            except IOError:
                WARNING(None, "Failed to establish connection with target, timeout. Restarting the target")
                return 'T'
            self.is_connected = True

        self.conn.settimeout(self.timeout)
        try:
            res = self.conn.recv(1)
        except IOError:
            WARNING(None, "Failed to recv data from target, timeout. Restarting the target")
            return 'T'

        INFO(1, None, None, "Done reading from UDS, result: %s" % res)
        return res.decode("utf-8")

    def recv_command(self):
        if sys.platform == "win32":
            return self.recv_command_win(self)
        elif "linux" in sys.platform:
            return self.recv_command_lin(self)
        else:
            ERROR("Invalid platform for DBI persistent mode")