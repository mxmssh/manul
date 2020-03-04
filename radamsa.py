from ctypes import *
import helper
import printing
import sys
import manul_utils

PY3 = sys.version_info[0] == 3


class RadamsaFuzzer(object):
    def __init__(self, seed):
        self.seed = seed
        self.lib = None
        if not PY3:
            printing.ERROR("Radamsa library is not supported in Python2")

    def load_library(self, library_path):
        self.lib = cdll.LoadLibrary(library_path)
        self.lib.radamsa_init()

        self.lib.radamsa.argtypes = [POINTER(c_ubyte), c_size_t, POINTER(c_ubyte), c_size_t, c_size_t]

    def radamsa_generate_output(self, radamsa_input):
        input_len = c_size_t(len(radamsa_input))
        output_len = helper.AFL_choose_block_len(helper.AFL_HAVOC_BLK_XL)
        radamsa_output = create_string_buffer(output_len)
        input_casted = cast(radamsa_input, POINTER(c_ubyte))
        output_casted = cast(radamsa_output, POINTER(c_ubyte))
        bytes_mutated = 0
        while bytes_mutated == 0: # we call it in cycle because sometimes radamsa mutates 0 bytes
            bytes_mutated = self.lib.radamsa(input_casted, input_len, output_casted, c_size_t(output_len),
                                             c_size_t(self.seed))
            self.seed += 1
        return bytearray(radamsa_output.raw[:bytes_mutated])