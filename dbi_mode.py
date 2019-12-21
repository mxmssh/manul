from printing import *
import os
import random
import win32pipe, win32file, pywintypes

COMMAND_CYCLE_FINISH = 'K'
COMMAND_CYCLE_START = 'P'
COMMAND_CRASH = 'C'
COMMAND_FINISH = 'F'

def gen_name():
    return random.randint(1, 999999999)

def gen_pipe_name_lin():
    pipe_name_in = "/usr/tmp/manul_dbi_pipe_in_%d" % gen_name()
    pipe_name_out = "/usr/tmp/manul_dbi_pipe_out_%d" % gen_name()
    return pipe_name_in, pipe_name_out

def gen_pipe_name_win():
    pipe_name_in = pipe_name_out =  r'\\.\pipe\\manul_dbi_pipe_in_%d' % gen_name()
    return pipe_name_in, pipe_name_out

def gen_pipe_name():
    if sys.platform == "win32":
        return gen_pipe_name_win()
    elif "linux" in sys.platform:
        return gen_pipe_name_lin()
    else:
        ERROR("Invalid platform for DBI persistent mode")

#TODO: remove tmp files on exit
#TODO (high priority): timeouts for all functions, since PIPES are blocking on Linux
class PipeHandler(object):
    def __init__(self):
        self.pipe_name_in, self.pipe_name_out = gen_pipe_name()
        self.pipe_in = None
        self.pipe_out = None
        self.is_pipe_connected = False

    def get_pipe_name(self):
        return self.pipe_name_in, self.pipe_name_out

    def setup_pipe_win(self):
        INFO(1, None, None, "Opening PIPE %s" % self.pipe_name_in)
        try:
            self.pipe_in = self.pipe_out = win32pipe.CreateNamedPipe(self.pipe_name_in,
                win32pipe.PIPE_ACCESS_DUPLEX,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                1, 65536, 65536,
                0,
                None)
        except OSError as e:
            ERROR("Failed to create named pipe %s! Error %s" % (self.pipe_name_in, e.message))

    def setup_pipe_lin(self):

        if os.path.exists(self.pipe_name_in): #unlikely
            os.remove(self.pipe_name_in)
        if os.path.exists(self.pipe_name_out):
            os.remove(self.pipe_name_out)

        try:
            os.mkfifo(self.pipe_name_in)
        except OSError as e:
            ERROR("Failed to create named pipe %s! Error %s" % (self.pipe_name_in, e.message))

        try:
            os.mkfifo(self.pipe_name_out)
        except OSError as e:
            ERROR("Failed to create named pipe %s! Error %s" % (self.pipe_name_out, e.message))

        INFO(1, None, None, "Opening PIPE %s" % self.pipe_name_in)
        self.pipe_in = open(self.pipe_name_in, 'wb', 0)
        INFO(1, None, None, "Opening PIPE %s" % self.pipe_name_out)
        self.pipe_out = open(self.pipe_name_out, 'rb', 0)

    def setup_pipe(self):
        if sys.platform == "win32":
            return self.setup_pipe_win()
        elif "linux" in sys.platform:
            return self.setup_pipe_lin()
        else:
            ERROR("Invalid platform for DBI persistent mode")

    def close_pipes_lin(self):
        if self.pipe_in:
            self.pipe_in.close()
        if self.pipe_out:
            self.pipe_out.close()

    def close_pipes_win(self):
        if self.pipe_in:
            win32file.CloseHandle(self.pipe_in)

    def close_pipes(self):
        if sys.platform == "win32":
            return self.close_pipes_win(self)
        elif "linux" in sys.platform:
            return self.close_pipes_lin(self)
        else:
            ERROR("Invalid platform for DBI persistent mode")

    def connect_pipe_win(self):
        if self.is_pipe_connected:
            self.is_pipe_connected = True
            return
        INFO(1, None, None, "Waiting for client to connect on pipe %s" % self.pipe_name_in)
        res = win32pipe.ConnectNamedPipe(self.pipe_in, None)
        INFO(1, None, None, "Client successfully connected to %s, res = %s" % (self.pipe_name_in, res))

    @staticmethod
    def send_command_win(self, command_str):
        INFO(1, None, None, "Sending %s" % command_str)
        res = win32file.WriteFile(self.pipe_in, command_str.encode("utf-8"))
        INFO(1, None, None, "Done sending command over PIPE %s" % self.pipe_in)
        return res

    @staticmethod
    def send_command_lin(self, command_str):
        INFO(1, None, None, "Sending %s into PIPE %s" % (command_str, self.pipe_in))
        res = self.pipe_in.write(command_str.encode("utf-8"))
        INFO(1, None, None, "Done sending command over PIPE %s" % self.pipe_in)
        return res

    def send_command(self, command_str):
        if sys.platform == "win32":
            return self.send_command_win(self, command_str)
        elif "linux" in sys.platform:
            return self.send_command_lin(self, command_str)
        else:
            ERROR("Invalid platform for DBI persistent mode")

    @staticmethod
    def recv_command_win(self):
        INFO(1, None, None, "Reading PIPE %s"  % self.pipe_out)
        try:
            res, res_str = win32file.ReadFile(self.pipe_in, 1)
        except OSError as e:
            ERROR("Failed to read from named pipe %s! Error %s" % (self.pipe_name_out, e.message))
        INFO(1, None, None, "Received %s, result code: %d" % (res_str, res))
        return res_str.decode("utf-8")

    @staticmethod
    def recv_command_lin(self):
        INFO(1, None, None, "Reading PIPE %s"  % self.pipe_out)
        res = self.pipe_out.read(1)
        INFO(1, None, None, "Received %s" % res)
        return res.decode("utf-8")

    def recv_command(self):
        if sys.platform == "win32":
            return self.recv_command_win(self)
        elif "linux" in sys.platform:
            return self.recv_command_lin(self)
        else:
            ERROR("Invalid platform for DBI persistent mode")