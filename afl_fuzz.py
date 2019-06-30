from helper import *
from collections import OrderedDict

#TODO: write unit test for each of this function
tokens_list = None

def bitflip_1bit(data, func_state): # for i in range((len(data)*8)):
    if not func_state:
        func_state = 0

    data[int(func_state / 8)] ^= (0x80 >> (func_state % 8))
    func_state += 1

    if func_state >= len(data) * 8:
        func_state = None # we are done here, lets switch to the next function

    return data, func_state

def bitflip_2bits(data, func_state): # for i in range((len(data)*7)):
    if not func_state:
        func_state = 0

    data[int(func_state / 7)] ^= (0xC0 >> (func_state % 7))

    func_state += 1

    if func_state >= len(data) * 7:
        func_state = None # we are done here, lets switch to the next function

    return data, func_state


def bitflip_4bits(data, func_state): # for i in range((len(data)*5)):
    if not func_state:
        func_state = 0

    data[int(func_state / 5)] ^= (0xF0 >> (func_state % 5))

    func_state += 1
    if func_state >= len(data) * 5:
        func_state = None # we are done here, lets switch to the next function

    return data, func_state


def byteflip_1(data, func_state): # for i in range((len(data))):
    if not func_state:
        func_state = 0

    data[func_state] ^= 0xFF

    func_state += 1

    if func_state >= len(data):
        func_state = None

    return data, func_state


def byteflip_2(data, func_state): # for i in range(1, ((len(data)))):
    if not func_state:
        func_state = 0

    if len(data) > 1:
        data[func_state] ^= 0xFF
        data[func_state + 1] ^= 0xFF
    else:
        return data, None # input too small for byteflipping

    func_state += 1

    if func_state + 1 >= len(data):
        func_state = None

    return data, func_state


def byteflip_4(data, func_state):
    if not func_state:
        func_state = 0

    if len(data) > 3:
        data[func_state] ^= 0xFF
        data[func_state + 1] ^= 0xFF
        data[func_state + 2] ^= 0xFF
        data[func_state + 3] ^= 0xFF
    else:
        return data, None # input too small for byteflipping

    func_state += 1

    if func_state + 3 >= len(data):
        func_state = None

    return data, func_state


set_arith_max = 35 # TODO: define it better!!!!
def mutate_byte_arithmetic(data, func_state):
    if not func_state:
        func_state = [0, False]

    # TODO: we have to check for could_be_bitflip()

    if func_state[1] == False:
        data[int(func_state[0] / set_arith_max)] = ((data[int(func_state[0] / set_arith_max)] + (func_state[0] % set_arith_max)) & 0xff)
    else:
        data[int(func_state[0] / set_arith_max)] = ((data[int(func_state[0] / set_arith_max)] - (func_state[0] % set_arith_max)) & 0xff)

    func_state[0] += 1

    if func_state[0] >= len(data) * set_arith_max:
        if func_state[1] == False:
            func_state = [0, False]
        else:
            func_state = None

    return data, func_state


def mutate_2bytes_arithmetic(data, func_state):
    if not func_state:
        func_state = (0, False)

    # TODO: we have to check for could_be_bitflip()
    #TODO: implement

    func_state[0] += 1

    if func_state[0] >= len(data) * set_arith_max:
        if func_state[1] == False:
            func_state = (0, True)
        else:
            func_state = None

    return data, func_state


def mutate_4bytes_arithmetic(data, func_state):
    if not func_state:
        func_state = [0, False]

    #TODO: we have to check for could_be_bitflip()
    #TODO: implement

    func_state[0] += + 1

    if func_state[0] >= len(data) * set_arith_max:
        if func_state[1] == False:
            func_state[0] = 0
            func_state[1] = True
        else:
            func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_1byte_interesting(data, func_state):
    if not func_state:
        func_state = [0, 0]

    interesting_value = interesting_8_Bit[func_state[1]]
    interesting_value = in_range_8(interesting_value)
    data = data[:func_state[0]] + interesting_value + data[func_state[0] + 1:]

    func_state[1] += 1

    if func_state[1] >= len(interesting_8_Bit):
        func_state[0] += 1

    if func_state[0] >= len(data):
        func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_2bytes_interesting(data, func_state):
    if not func_state:
        func_state = [0, 0, False]

    interesting_value = in_range_16(interesting_16_Bit[func_state[1]])

    if func_state[2] == False:
        data = data[:func_state[0]] + interesting_value + data[func_state[0] + 2:]
    else:
        swapped_value = swap_16(interesting_value)
        data = data[:func_state[0]] + swapped_value + data[func_state[0] + 2:]

    func_state[1] += 1

    if func_state[1] >= len(interesting_16_Bit):
        func_state[0] += 1

    if func_state[0] >= len(data):
        if func_state[2] == False:
            func_state = (0, 0, True)
        else:
            func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_4bytes_interesting(data, func_state):
    if not func_state:
        func_state = [0, 0, False]
    interesting_value = in_range_32(interesting_32_Bit[func_state[1]])

    if func_state[2] == False:
        data = data[:func_state[0]] + interesting_value + data[func_state[0] + 4:]
    else:
        swapped_value = swap_32(interesting_value)
        data = data[:func_state[0]] + swapped_value + data[func_state[0] + 4:]

    func_state[1] += 1

    if func_state[1] >= len(interesting_32_Bit):
        func_state[0] += 1

    if func_state[0] >= len(data):
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


#TODO: auto-create dictionary from binary
#TODO: afl has this also https://github.com/mirrorer/afl/blob/2fb5a3482ec27b593c57258baae7089ebdc89043/afl-fuzz.c#L5123
def dictionary_overwrite(data, func_state):
    global tokens_list
    if not func_state:
        func_state = [0, 0] # first is an index in tokens_list, second is an index in data

    data_len = len(data)
    token = tokens_list[func_state[0]]
    place = func_state[1]

    data = data[:place] + token + data[place + len(token):]
    func_state[1] += 1

    if place >= data_len- len(token):
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            func_state = None

    return data, func_state


def dictionary_insert(data, func_state):
    global tokens_list
    if not func_state:
        func_state = [0, 0] # first is an index in tokens_list, second is an index in data

    data_len = len(data)

    token = tokens_list[func_state[0]]
    place = func_state[1]

    data = data[:place] + token + data[place:]
    func_state[1] += 1

    if place >= data_len:
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            func_state = None

    return data, func_state


def havoc(data, func_state):
    #TODO: implement
    return data, None


def trim(data, func_state):
    #TODO: implement
    return data, None


# TODO: extras_ao, havoc, splice
stages_list = {"flip1", "flip2", "flip4", "flip8", "flip16", "flip32", "arith8", "arith16", "arith32", "interest8",
               "interest16", "interest32", "extras_uo", "extras_ui", "havoc", "splice"}


class AFLFuzzer(object):
    def __init__(self, user_tokens_dict):
        global tokens_list
        self.possible_stages = OrderedDict()

        self.list_of_functions = [bitflip_1bit, bitflip_2bits, bitflip_4bits,
                                  byteflip_1, byteflip_2, byteflip_4,
                                  mutate_byte_arithmetic, mutate_2bytes_arithmetic, mutate_4bytes_arithmetic,
                                  mutate_1byte_interesting, mutate_2bytes_interesting, mutate_4bytes_interesting,
                                  dictionary_overwrite, dictionary_insert, havoc, trim]
        self.current_function = self.list_of_functions[0]
        self.current_result = None
        self.current_function_id = 0
        tokens_list = user_tokens_dict

    def mutate(self, data):

        data, self.current_result = self.current_function(data, self.current_result) # we save and send result in the next call
        if not self.current_result:
            self.current_function_id += 1
            self.current_function = self.list_of_functions[self.current_function_id % len(self.list_of_functions)]
        return data