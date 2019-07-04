from helper import *
from collections import OrderedDict
from manul_utils import  *

#TODO: write unit test for each of this function.
tokens_list = None
tokens_list_length = None

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


def mutate_byte_arithmetic(data, func_state):
    if not func_state:
        func_state = [0, 0, False]

    # TODO: we have to check for could_be_bitflip()

    if func_state[2] == False:
        val = data[func_state[0]] + func_state[1]
    else:
        val = data[func_state[0]] - func_state[1]

    store_8(data, func_state[0], val)

    func_state[1] += 1
    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] >= len(data):
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


def mutate_2bytes_arithmetic(data, func_state):
    data_len = len(data)
    if data_len < 2:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    # TODO: we have to check for could_be_bitflip()
    val = load_16(data, func_state[0])

    if func_state[2] == False:
        val += func_state[1]
    else:
        val -= func_state[1]

    store_16(data, func_state[0], val)

    func_state[1] += 1
    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 1 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


def mutate_4bytes_arithmetic(data, func_state):
    data_len = len(data)
    if data_len < 4:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    # TODO: we have to check for could_be_bitflip()
    val = load_32(data, func_state[0])

    if func_state[2] == False:
        val += func_state[1]
    else:
        val -= func_state[1]

    store_32(data, func_state[0], val)

    func_state[1] += 1
    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 3 >= len(data):
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_1byte_interesting(data, func_state):
    if not func_state:
        func_state = [0, 0]

    interesting_value = interesting_8_Bit[func_state[1]]
    data[func_state[0]] = in_range_8(interesting_value)

    func_state[1] += 1

    if func_state[1] >= len(interesting_8_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] >= len(data):
        func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_2bytes_interesting(data, func_state):
    data_len = len(data)
    if data_len < 2:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    interesting_value = in_range_16(interesting_16_Bit[func_state[1]])

    if func_state[2]:
        interesting_value = swap_16(interesting_value)

    store_16(data, func_state[0], interesting_value)

    func_state[1] += 1

    if func_state[1] >= len(interesting_16_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 1 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_4bytes_interesting(data, func_state):
    data_len = len(data)
    if data_len < 4:
        return data, None

    if not func_state:
        func_state = [0, 0, False]
    interesting_value = in_range_32(interesting_32_Bit[func_state[1]])

    if func_state[2]:
        interesting_value = swap_32(interesting_value)

    store_32(data, func_state[0], interesting_value)

    func_state[1] += 1

    if func_state[1] >= len(interesting_32_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 3 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            func_state = None

    return data, func_state


#TODO: auto-create dictionary from binary
#TODO: afl has this also https://github.com/mirrorer/afl/blob/2fb5a3482ec27b593c57258baae7089ebdc89043/afl-fuzz.c#L5123
def dictionary_overwrite(data, func_state):
    global tokens_list, tokens_list_length
    if tokens_list_length <= 0:
        return data, None
    if not func_state:
        func_state = [0, 0] # first is an index in tokens_list, second is an index in data

    data_len = len(data)
    token = tokens_list[func_state[0]]
    place = func_state[1]

    if data_len <= len(token):
        return data, None

    data = data[:place] + bytearray(token) + data[place + len(token):]
    func_state[1] += 1

    if place >= data_len - len(token):
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            func_state = None

    return data, func_state


def dictionary_insert(data, func_state):
    global tokens_list, tokens_list_length
    if tokens_list_length <= 0:
        return data, None

    if not func_state:
        func_state = [0, 0] # first is an index in tokens_list, second is an index in data

    data_len = len(data)

    token = tokens_list[func_state[0]]
    place = func_state[1]

    data = data[:place] + bytearray(token) + data[place:]
    func_state[1] += 1

    if place >= data_len:
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            func_state = None

    return data, func_state


def havoc_bitflip(data):
    value_to_flip = RAND(len(data) * 8)
    data, res = bitflip_1bit(data, value_to_flip)
    return data


def havoc_interesting_byte(data):
    value_to_change = RAND(len(data))
    interesting_value_index = RAND(len(interesting_8_Bit))

    func_state = [value_to_change, interesting_value_index]
    data, state = mutate_1byte_interesting(data, func_state)
    return data


def havoc_interesting_2bytes(data):
    data_len = len(data)
    if data_len < 2:
        return data
    value_to_change = RAND(data_len - 1) # substract 1 to make sure we have space for 2 bytes
    interesting_value_index = RAND(len(interesting_16_Bit))
    swap = RAND(2) # is swap?

    func_state = [value_to_change, interesting_value_index, swap]
    data, state = mutate_2bytes_interesting(data, func_state)
    return data


def havoc_interesting_4bytes(data):
    data_len = len(data)
    if data_len < 4:
        return data
    value_to_change = RAND(len(data) - 3) # substract 1 to make sure we have space for 2 bytes
    interesting_value_index = RAND(len(interesting_32_Bit))
    swap = RAND(2) # is swap?

    func_state = [value_to_change, interesting_value_index, swap]
    data, state = mutate_4bytes_interesting(data, func_state)
    return data


def havoc_randomly_add(data): # similar to mutate_byte_arithmetic but a bit faster
    value_to_change = RAND(len(data))
    data[value_to_change] = in_range_8(data[value_to_change] + 1 + RAND(AFL_ARITH_MAX))
    return data


def havoc_randomly_substract(data): # similar to mutate_byte_arithmetic but a bit faster
    value_to_change = RAND(len(data))
    data[value_to_change] = in_range_8(data[value_to_change] - (1 + RAND(AFL_ARITH_MAX)))
    return data


def havoc_randomly_add_2bytes(data): # similar to mutate_byte_arithmetic but a bit faster
    data_len = len(data)
    if data_len < 2:
        return data
    func_state = [RAND(data_len - 1), RAND(AFL_ARITH_MAX), True] # pos, value, is_sub
    data, func_state = mutate_2bytes_arithmetic(data, func_state)
    return data


def havoc_randomly_substract_2bytes(data): # similar to mutate_byte_arithmetic but a bit faster
    data_len = len(data)
    if data_len < 2:
        return data
    func_state = [RAND(data_len - 1), RAND(AFL_ARITH_MAX), False] # pos, value, is_sub
    data, func_state = mutate_2bytes_arithmetic(data, func_state)
    return data


def havoc_randomly_add_4bytes(data): # similar to mutate_byte_arithmetic but a bit faster
    data_len = len(data)
    if data_len < 4:
        return data
    func_state = [RAND(data_len - 3), RAND(AFL_ARITH_MAX), True] # pos, value, is_sub
    data, func_state = mutate_4bytes_arithmetic(data, func_state)
    return data


def havoc_randomly_substract_4bytes(data): # similar to mutate_byte_arithmetic but a bit faster
    data_len = len(data)
    if data_len < 4:
        return data
    func_state = [RAND(data_len - 3), RAND(AFL_ARITH_MAX), False] # pos, value, is_sub
    data, func_state = mutate_4bytes_arithmetic(data, func_state)
    return data


def havoc_set_randomly(data):
    pos = RAND(len(data))
    data[pos] = in_range_8(data[pos] ^ (1 + RAND(255)));
    return data


def havoc_remove_randomly_block(data):
    data_len = len(data)
    if data_len <= 2:
        return data

    len_to_remove = AFL_choose_block_len(data_len)
    pos = RAND(data_len)
    data = data[:pos] + data[pos+len_to_remove:]
    return data


def prepare_block(data):
    actually_clone = RAND(4)
    data_len = len(data)
    if data_len < 4: # doesn't make sense to clone blocks less than 4 bytes
        return 0,0,0
    if actually_clone:
        clone_len = AFL_choose_block_len(data_len)
        clone_from = RAND(data_len - clone_len + 1 )
    else:
        clone_len = AFL_choose_block_len(AFL_HAVOC_BLK_XL)
        clone_from = 0
    clone_to = RAND(data_len)

    if actually_clone:
        block = data[clone_from:clone_from + clone_len]
    else:
        use_data_block = RAND(2)
        if use_data_block:
            block_start = RAND(data_len)
            block = data[block_start:block_start+clone_len]
        else:
            block = [RAND(256)] * clone_len # TODO: check if it is actually correct implementation
            block = bytearray(block)
    return block, clone_to, clone_len


# insert random block
def havoc_clone_randomly_block(data):
    block, clone_to, clone_len = prepare_block(data)
    if clone_len == 0:
        return data
    data = data[:clone_to] + block + data[clone_to:]
    return data


# overwrite random block
def havoc_overwrite_randomly_block(data):
    block, clone_to, clone_len = prepare_block(data)
    if clone_len == 0:
        return data
    data = data[:clone_to] + block + data[clone_to+clone_len:]
    return data


# overwrite from dict
def havoc_overwrite_with_dict(data):
    func_state = [RAND(len(tokens_list)), RAND(len(data))]
    dictionary_overwrite(data, func_state)
    return data


# overwrite from dict
def havoc_insert_with_dict(data):
    func_state = [RAND(len(tokens_list)), RAND(len(data))]
    dictionary_insert(data, func_state)
    return data

#TODO: https://github.com/mirrorer/afl/blob/2fb5a3482ec27b593c57258baae7089ebdc89043/afl-fuzz.c#L6478
def havoc(data, func_state):
    if not func_state:
        func_state = 0

    # havoc_randomly used twice to increase chances
    func_to_choose = [havoc_bitflip, havoc_interesting_byte, havoc_interesting_2bytes, havoc_interesting_4bytes,
                      havoc_randomly_add, havoc_randomly_substract, havoc_randomly_add_2bytes,
                      havoc_randomly_substract_2bytes, havoc_randomly_add_4bytes, havoc_randomly_substract_4bytes,
                      havoc_set_randomly, havoc_remove_randomly_block, havoc_remove_randomly_block, havoc_clone_randomly_block,
                      havoc_overwrite_randomly_block, havoc_overwrite_with_dict, havoc_insert_with_dict]

    use_stacking = 1 << 1 + RAND(AFL_HAVOC_STACK_POW2)
    for i in range (0, use_stacking):
        method = RAND(len(func_to_choose))
        # randomly select one of the available methods
        data = func_to_choose[method](data)
    func_state += 1

    if func_state >= len(data): # TODO: havoc length should be calculated based on performance not on data length
        func_state = None

    return data, func_state


def splice(data, list_of_files, queue_path, func_state):
    data_len = len(data)
    if data_len <= 2:
        return data, None

    if len(list_of_files) <= 1:
        return data, None

    if not func_state:
        func_state = 0

    # pick up random file from queue and splice with it
    file_id = RAND(len(list_of_files))
    if isinstance(list_of_files[file_id], tuple):
        file_name = list_of_files[file_id][1]
    else:
        file_name = list_of_files[file_id]
    picked_file_name = queue_path + "/" + file_name
    if "manul" not in picked_file_name and "_mutated" not in picked_file_name:
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None)

    content_target = extract_content(picked_file_name)
    content_target_len = len(content_target)

    if content_target_len < 2 or is_bytearrays_equal(data, content_target):
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None)

    f_diff, l_diff = locate_diffs(data, content_target, MIN(data_len, content_target_len))

    if f_diff == 0 or l_diff < 2 or f_diff == l_diff:
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None)

    split_last_byte = f_diff + RAND(l_diff - f_diff)
    block = data[f_diff:f_diff+split_last_byte]
    content_target = content_target[:f_diff] + block + content_target[f_diff+split_last_byte]
    data = content_target

    data, res = havoc(data, 0)

    if func_state > SPLICE_CYCLES:
        return data, None

    return data, func_state

class AFLFuzzer(object):
    def __init__(self, user_tokens_dict, queue_path):
        global tokens_list, tokens_list_length
        self.possible_stages = OrderedDict()

        self.list_of_functions = [bitflip_1bit, bitflip_2bits, bitflip_4bits,
                                  byteflip_1, byteflip_2, byteflip_4,
                                  mutate_byte_arithmetic, mutate_2bytes_arithmetic, mutate_4bytes_arithmetic,
                                  mutate_1byte_interesting, mutate_2bytes_interesting, mutate_4bytes_interesting,
                                  dictionary_overwrite, dictionary_insert, havoc, splice]
        self.current_function = self.list_of_functions[0]
        self.current_result = None
        self.current_function_id = 0
        self.total_func_count = len(self.list_of_functions)
        self.queue_path = queue_path
        tokens_list = user_tokens_dict
        tokens_list_length = len(tokens_list)

    def mutate(self, data, list_of_files):
        if len(data) <= 0:
            return data

        if self.current_function_id % self.total_func_count == self.total_func_count - 1: # if splice
            data, self.current_result = self.current_function(data, list_of_files, self.queue_path, self.current_result)
        else:
            data, self.current_result = self.current_function(data, self.current_result)

        if not self.current_result:
            self.current_function_id += 1
            self.current_function = self.list_of_functions[self.current_function_id % len(self.list_of_functions)]
        return data