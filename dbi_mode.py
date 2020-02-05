from printing import *
import os
import random
import socket
if sys.platform == "win32":
    import win32pipe, win32file, pywintypes
from select import select

COMMAND_CYCLE_FINISH = 'K'
COMMAND_CYCLE_START = 'P'
COMMAND_CRASH = 'C'
COMMAND_FINISH = 'F'

def gen_socket_name_lin():
    return "/usr/tmp/manul_uds_socket_%d" % random.randint(1, 999999999)

def gen_socket_name_win():
    return r'manul_uds_socket_%d' % random.randint(1, 999999999)

def gen_socket_name():
    if sys.platform == "win32":
        return gen_socket_name_win()
    elif "linux" in sys.platform:
        return gen_socket_name_lin()
    else:
        ERROR("Invalid platform for DBI persistent mode")

#TODO: remove tmp files on exit
#TODO (high priority): timeouts for pipes on Windows (blocking situations)
class SocketHandler(object):
    def __init__(self, timeout):
        self.socket_name = gen_socket_name()
        self.sock = None
        self.conn = None
        self.is_connectected = False
        self.timeout = timeout

    def get_socket_name(self):
        return self.socket_name

    def setup_socket_win(self):
        return None

    def setup_socket_lin(self):
        # Make sure the socket does not already exist
        try:
            os.unlink(self.socket_name)
        except OSError:
            if os.path.exists(self.socket_name):
                ERROR("Socket already exists %s" % self.socket_name)

        # Create a UDS socket
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        # Bind the socket to the port
        INFO(1, None, None, "Starting UDS on %s" % self.socket_name)
        self.sock.bind(self.socket_name)

        # Listen for incoming connections
        self.sock.listen(1)
        self.sock.settimeout(self.timeout) # TODO: pass timeout from config
        self.is_connectected = False

    def setup_socket(self):
        if sys.platform == "win32":
            return self.setup_socket_win()
        elif "linux" in sys.platform:
            return self.setup_socket_lin()
        else:
            ERROR("Invalid platform for DBI persistent mode")

    def close_socket_lin(self):
        self.sock.close()
        os.unlink(self.socket_name)

    def close_socket_win(self):
        return None

    def close_socket(self):
        if sys.platform == "win32":
            return self.close_socket_win()
        elif "linux" in sys.platform:
            return self.close_socket_lin()
        else:
            ERROR("Invalid platform for DBI persistent mode")

    @staticmethod
    def send_command_win(self, command_str):
        return None

    @staticmethod
    def send_command_lin(self, command_str):
        INFO(1, None, None, "Sending %s into socket %s" % (command_str, self.socket_name))
        if not self.conn:
            ERROR("Wrong recv/send command order, connection is not yet established!")
        self.conn.send(command_str.encode("utf-8"))
        INFO(1, None, None, "Done sending command in UDS %s" % self.socket_name)

    def send_command(self, command_str):
        if sys.platform == "win32":
            return self.send_command_win(self, command_str)
        elif "linux" in sys.platform:
            return self.send_command_lin(self, command_str)
        else:
            ERROR("Invalid platform for DBI persistent mode")

    @staticmethod
    def recv_command_win(self):
        return

    @staticmethod
    def recv_command_lin(self):
        INFO(1, None, None, "Reading from UDS %s"  % self.socket_name)
        res = None

        if not self.is_connectected:
            # Wait for a connection
            try:
                self.conn, client_address = self.sock.accept()
            except IOError:
                WARNING(None, "Failed to establish connection with target, timeout. Restarting the target")
                return 'T'
            self.is_connectected = True

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