from printing import *
import os
import random

COMMAND_CYCLE_FINISH = 'K'
COMMAND_CYCLE_START = 'P'
COMMAND_CRASH = 'C'
COMMAND_FINISH = 'F'

def gen_name():
    return random.randint(1, 999999999)

def gen_pipe_name():
    pipe_name_in = "/usr/tmp/manul_dbi_pipe_in_%d" % gen_name()
    pipe_name_out = "/usr/tmp/manul_dbi_pipe_out_%d" % gen_name()
    return pipe_name_in, pipe_name_out

#TODO: remove tmp files on exit
#TODO: timeouts for all functions, since PIPES are blocking
class PipeHandler(object):
    def __init__(self):
        self.pipe_name_in, self.pipe_name_out = gen_pipe_name()
        self.pipe = None
        self.pipe_open = False
        self.pipe_in = None
        self.pipe_out = None

    def get_pipe_name(self):
        return self.pipe_name_in, self.pipe_name_out

    def setup_pipe_win(self):
        return

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

    def close_pipes(self):
        if self.pipe_in:
            self.pipe_in.close()
        if self.pipe_out:
            self.pipe_out.close()

    @staticmethod
    def send_command_win(self, command_str):
        return command_str

    @staticmethod
    def send_command_lin(self, command_str):
        INFO(1, None, None, "Sending %s" % command_str)
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
        res = self.pipe.read()
        return res

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