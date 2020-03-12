#   Manul - AFL mutator ported to Python
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

from printing import *
import ast

#TODO: write unit test for each of this function.
tokens_list = None
tokens_list_length = None

def bitflip_1bit(data, func_state): # for i in range((len(data)*8)):
    if not func_state:
        func_state = 0

    if func_state >= len(data) * 8:
        return data, None # we are done here, lets switch to the next function

    data[int(func_state / 8)] ^= (0x80 >> (func_state % 8))
    func_state += 1

    return data, func_state

def bitflip_2bits(data, func_state): # for i in range((len(data)*7)):
    if not func_state:
        func_state = 0

    if func_state >= len(data) * 7:
        return data, None# we are done here, lets switch to the next function

    data[int(func_state / 7)] ^= (0xC0 >> (func_state % 7))

    func_state += 1

    return data, func_state


def bitflip_4bits(data, func_state): # for i in range((len(data)*5)):
    if not func_state:
        func_state = 0

    if func_state >= len(data) * 5:
        return data, None # we are done here, lets switch to the next function

    data[int(func_state / 5)] ^= (0xF0 >> (func_state % 5))

    func_state += 1

    return data, func_state


def byteflip_1(data, func_state): # for i in range((len(data))):
    if not func_state:
        func_state = 0

    if func_state >= len(data):
        return data, None # we are done here, lets switch to the next function

    data[func_state] ^= 0xFF

    func_state += 1

    return data, func_state


def byteflip_2(data, func_state): # for i in range(1, ((len(data)))):
    if not func_state:
        func_state = 0

    if func_state + 1 >= len(data):
        return data, None # we are done here, lets switch to the next function

    if len(data) > 1:
        data[func_state] ^= 0xFF
        data[func_state + 1] ^= 0xFF
    else:
        return data, None # input too small for byteflipping

    func_state += 1



    return data, func_state


def byteflip_4(data, func_state):
    if not func_state:
        func_state = 0

    if func_state + 3 >= len(data):
        return data, None

    if len(data) > 3:
        data[func_state] ^= 0xFF
        data[func_state + 1] ^= 0xFF
        data[func_state + 2] ^= 0xFF
        data[func_state + 3] ^= 0xFF
    else:
        return data, None # input too small for byteflipping

    func_state += 1


    return data, func_state


def mutate_byte_arithmetic(data, func_state):
    if not func_state:
        func_state = [0, 0, False]

    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] >= len(data):
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            return data, None

    # TODO: we have to check for could_be_bitflip()

    if func_state[2] == False:
        val = data[func_state[0]] + func_state[1]
    else:
        val = data[func_state[0]] - func_state[1]

    store_8(data, func_state[0], val)

    func_state[1] += 1

    return data, func_state


def mutate_2bytes_arithmetic(data, func_state):
    data_len = len(data)
    if data_len < 2:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 1 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            return data, None

    # TODO: we have to check for could_be_bitflip()
    val = load_16(data, func_state[0])

    if func_state[2] == False:
        val += func_state[1]
    else:
        val -= func_state[1]

    store_16(data, func_state[0], val)

    func_state[1] += 1

    return data, func_state


def mutate_4bytes_arithmetic(data, func_state):
    data_len = len(data)
    if data_len < 4:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    if func_state[1] > AFL_ARITH_MAX:
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 3 >= len(data):
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            return data, None

    # TODO: we have to check for could_be_bitflip()
    val = load_32(data, func_state[0])

    if func_state[2] == False:
        val += func_state[1]
    else:
        val -= func_state[1]

    store_32(data, func_state[0], val)

    func_state[1] += 1

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_1byte_interesting(data, func_state):
    if not func_state:
        func_state = [0, 0]

    if func_state[1] >= len(interesting_8_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] >= len(data):
        return data, None

    interesting_value = interesting_8_Bit[func_state[1]]

    data[func_state[0]] = in_range_8(interesting_value)

    func_state[1] += 1

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_2bytes_interesting(data, func_state):
    data_len = len(data)
    if data_len < 2:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    if func_state[1] >= len(interesting_16_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 1 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            return data, None

    interesting_value = in_range_16(interesting_16_Bit[func_state[1]])

    if func_state[2]:
        interesting_value = swap_16(interesting_value)

    store_16(data, func_state[0], interesting_value)

    func_state[1] += 1

    return data, func_state


# TODO: implement is_not_bitflip and is_not_arithmetic
def mutate_4bytes_interesting(data, func_state):
    data_len = len(data)
    if data_len < 4:
        return data, None

    if not func_state:
        func_state = [0, 0, False]

    if func_state[1] >= len(interesting_32_Bit):
        func_state[0] += 1
        func_state[1] = 0

    if func_state[0] + 3 >= data_len:
        if func_state[2] == False:
            func_state = [0, 0, True]
        else:
            return data, None

    interesting_value = in_range_32(interesting_32_Bit[func_state[1]])

    if func_state[2]:
        interesting_value = swap_32(interesting_value)

    store_32(data, func_state[0], interesting_value)

    func_state[1] += 1

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

    if data_len < len(token):
        return data, None

    if place >= data_len - len(token):
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            return data, None

    data = data[:place] + bytearray(token) + data[place + len(token):]
    func_state[1] += 1

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

    if place >= data_len:
        func_state[0] += 1 # take the next token
        func_state[1] = 0

        if func_state[0] >= len(tokens_list):
            return data, None

    data = data[:place] + bytearray(token) + data[place:]
    func_state[1] += 1

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

    len_to_remove = AFL_choose_block_len(data_len - 1)
    pos = RAND(data_len - len_to_remove + 1)
    data = data[:pos] + data[pos+len_to_remove:]
    return data


def prepare_block(data):
    actually_clone = RAND(4)
    data_len = len(data)

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
    data, func_state = dictionary_overwrite(data, func_state)
    return data


# overwrite from dict
def havoc_insert_with_dict(data):
    func_state = [RAND(len(tokens_list)), RAND(len(data))]
    data, func_state = dictionary_insert(data, func_state)
    return data

#TODO: https://github.com/mirrorer/afl/blob/2fb5a3482ec27b593c57258baae7089ebdc89043/afl-fuzz.c#L6478
def havoc(data, func_state, max_havoc_cycles):
    if not func_state:
        func_state = 0

    if func_state >= max_havoc_cycles:
        return data, None

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

    return data, func_state


def splice(data, list_of_files, queue_path, func_state, max_havoc_cycles):
    data_len = len(data)
    if data_len <= 2:
        return data, None

    if len(list_of_files) <= 1:
        return data, None

    if not func_state:
        func_state = 0

    if func_state > SPLICE_CYCLES:
        return data, None

    # pick up random file from queue and splice with it
    file_id = RAND(len(list_of_files))
    if isinstance(list_of_files[file_id], tuple):
        file_name = list_of_files[file_id][1]
        picked_file_name = queue_path + "/" + file_name
    else:
        # TODO: currently we don't touch original files (which is not right I believe)
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None, max_havoc_cycles)

    content_target = extract_content(picked_file_name)
    content_target_len = len(content_target)

    if content_target_len < 2 or is_bytearrays_equal(data, content_target):
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None, max_havoc_cycles)

    f_diff, l_diff = locate_diffs(data, content_target, MIN(data_len, content_target_len))

    if l_diff < 2 or f_diff == l_diff: # afl has f_diff == 0 but I believe we want to start with 0
        del list_of_files[file_id]
        return splice(data, list_of_files, queue_path, None, max_havoc_cycles)

    split_last_byte = f_diff + RAND(l_diff - f_diff)
    block = data[f_diff:f_diff+split_last_byte]
    content_target = content_target[:f_diff] + block + content_target[f_diff+split_last_byte:]
    data = content_target

    data, res = havoc(data, 0, max_havoc_cycles)

    return data, func_state

# calculate AFL-like performance score
def calculate_perf_score(exec_per_sec, avg_exec_per_sec, bitmap_size, avg_bitmap_size, handicap):
    perf_score = 100
    if exec_per_sec * 0.1 > avg_exec_per_sec: perf_score = 10
    elif exec_per_sec * 0.25 > avg_exec_per_sec: perf_score = 25
    elif exec_per_sec * 0.5 > avg_exec_per_sec: perf_score = 50
    elif exec_per_sec * 0.75 > avg_exec_per_sec: perf_score = 75
    elif exec_per_sec * 4 < avg_exec_per_sec: perf_score = 300
    elif exec_per_sec < avg_exec_per_sec: perf_score = 200
    elif exec_per_sec * 2 < avg_exec_per_sec: perf_score = 150

    if bitmap_size * 0.3 > avg_bitmap_size: perf_score *= 3;
    elif bitmap_size * 0.5 > avg_bitmap_size: perf_score *= 2;
    elif bitmap_size * 0.75 > avg_bitmap_size: perf_score *= 1.5;
    elif bitmap_size * 3 < avg_bitmap_size: perf_score *= 0.25;
    elif bitmap_size * 2 < avg_bitmap_size: perf_score *= 0.5;
    elif bitmap_size * 1.5 < avg_bitmap_size: perf_score *= 0.75;

    if handicap > 4:
        perf_score *= 4
        # TODO: handicap -= 4
    elif handicap:
        per_score *= 2
        #TODO handicap -= 1

    return perf_score


# Calculates maximum number of cycles
def get_havoc_cycles(exec_per_sec, perf_score, splice):
    havoc_div = 1
    if exec_per_sec < 20:
        havoc_div = 10
    elif exec_per_sec >= 20 and exec_per_sec < 50:
        havoc_div = 5
    elif exec_per_sec >= 50 and exec_per_sec < 100:
        havoc_div = 2

    # from AFL source code
    # stage_max = (doing_det ? HAVOC_CYCLES_INIT : HAVOC_CYCLES) * perf_score / havoc_div / 100;
    # doing_det is -d flag of afl (not supported)

    if splice:
        stage_max = AFL_SPLICE_HAVOC * perf_score / havoc_div / 100;
    else:
        stage_max = AFL_HAVOC_CYCLES * perf_score / havoc_div / 100


    # if not splice:
        # TODO: if (queued_paths != havoc_queued)
        # if we see more paths found by havoc spend more time
        #if perf_score <= AFL_HAVOC_MAX_MULT * 100:
        #    stage_max *= 2
        #    perf_score *= 2

    return stage_max, perf_score


class AFLFuzzer(object):
    def __init__(self, user_tokens_dict, queue_path, file_name):
        global tokens_list, tokens_list_length
        self.possible_stages = OrderedDict()

        # The order of this functions is super important. Splice must be the last and
        # havoc must be before splice.
        self.list_of_functions = [bitflip_1bit, bitflip_2bits, bitflip_4bits,
                                  byteflip_1, byteflip_2, byteflip_4,
                                  mutate_byte_arithmetic, mutate_2bytes_arithmetic, mutate_4bytes_arithmetic,
                                  mutate_1byte_interesting, mutate_2bytes_interesting, mutate_4bytes_interesting,
                                  dictionary_overwrite, dictionary_insert, havoc, splice]
        self.current_function = self.list_of_functions[0]
        self.file_name = file_name
        self.current_result = None
        self.current_function_id = 0
        self.total_func_count = len(self.list_of_functions)
        self.queue_path = queue_path
        tokens_list = user_tokens_dict
        tokens_list_length = len(tokens_list)
        self.new_havoc_cycle = True
        self.perf_score = 100
        self.orig_perf_score = 100


    def save_state(self, output_path):
        # we need to save current_function_id and current_result
        fd = open(output_path + "/afl_state_%s" % self.file_name, 'w')
        if not fd:
            WARNING(None, "Failed to save state of the AFLFuzzer for %s" % self.file_name)
        fd.write("%d %s" % (self.current_function_id, str(self.current_result)))
        fd.close()


    def restore_state(self, output_path):
        # we need to load current_function_id and current_result
        INFO(1, None, None, "Loading AFL state from %s"  % (output_path + "/afl_state_" + self.file_name))
        try:
            fd = open(output_path + "/afl_state_%s" % self.file_name, 'r')
        except FileNotFoundError:
            WARNING(None, "Unable to find %s file" % output_path + "/afl_state_%s" % self.file_name)
            return # it can happen when we just found the file and user raised ctrl+c
        if not fd:
            WARNING(None, "Failed to load state of the AFLFuzzer for %s, will start from the beginning" % self.file_name)
            return
        content = fd.read()
        function_id = content[:content.find(" ")]
        self.current_function_id = int(function_id)
        state = content[content.find(" ")+1:]
        if "[" in state:
            state = ast.literal_eval(state)
        elif "None" in state:
            state = None
        else:
            state = int(state)
        self.current_result = state
        self.current_function = self.list_of_functions[self.current_function_id % len(self.list_of_functions)]
        fd.close()
        INFO(1, None, None, "%s %s %s" % (self.file_name, self.current_result, self.current_function))


    def mutate(self, data, list_of_files, exec_per_sec, avg_exec_per_sec, bitmap_size,
               avg_bitmap_size, handicap):
        if len(data) <= 0:
            return data

        if not self.current_result:
            self.current_function = self.list_of_functions[self.current_function_id % len(self.list_of_functions)]

        INFO(1, None, None, "Running %s stage of AFL mutator" % self.current_function)

        if (self.current_function_id % self.total_func_count) == (self.total_func_count - 1): # if splice

            if self.new_havoc_cycle:
                self.havoc_max_stages, self.perf_score = get_havoc_cycles(exec_per_sec, self.perf_score, True)
                self.new_havoc_cycle = False

            # FYI: we are sending copy of list_of_files instead of actual list_of_files in slice
            data, self.current_result = self.current_function(data, list(list_of_files),
                                                              self.queue_path,
                                                              self.current_result,
                                                              self.havoc_max_stages)
            if not self.current_result:
                self.new_havoc_cycle = True
                self.perf_score = self.orig_perf_score

        elif (self.current_function_id % self.total_func_count) == (self.total_func_count - 2): # if havoc
            if self.new_havoc_cycle:
                self.perf_score = calculate_perf_score(exec_per_sec, avg_exec_per_sec, bitmap_size,
                                                   avg_bitmap_size, handicap)
                self.orig_perf_score = self.perf_score
                self.havoc_max_stages, self.perf_score = get_havoc_cycles(exec_per_sec, self.perf_score, False)
                self.new_havoc_cycle = False
            data, self.current_result = self.current_function(data, self.current_result,
                                                              self.havoc_max_stages)
            if not self.current_result:
                self.new_havoc_cycle = True
                self.perf_score = self.orig_perf_score
        else:
            data, self.current_result = self.current_function(data, self.current_result)

        if not self.current_result:
            self.current_function_id += 1

        return data