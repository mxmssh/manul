#   Manul - unit tests
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

from afl_fuzz import *
import copy
import radamsa
import sys

def test_bitflip(data, iteration_id):

    expected_output_1_bit = ["bebebebebebebebebe", "bebebebe", "bebe", "be"]
    expected_output_2_bits = ["c0c0c0c0c0c0c0c0c0", "c0c0c0c0", "c0c0", "c0"]
    expected_output_4_bits = ["e4e4e4e4e4e4e4e4e4", "e4e4e4e4", "e4e4", "e4"]

    original_data = copy.copy(data)  # sic!
    last_str = None
    res = None
    while True:
        data, res = bitflip_1bit(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_1_bit[iteration_id]:
        print("bitflip1 failed, output is %s" % last_str)
    else:
        print("bitflip1 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None
    while True:
        data, res = bitflip_2bits(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_2_bits[iteration_id]:
        print("bitflip2 failed, output is %s" % last_str)
    else:
        print("bitflip2 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None
    while True:
        data, res = bitflip_4bits(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_4_bits[iteration_id]:
        print("bitflip4 failed, output is %s" % last_str)
    else:
        print("bitflip4 succeeded")

    return True


def test_byteflip(data, iteration_id):
    res = None
    expected_output_1_byte = ["bebebebebebebebebe", "bebebebe", "bebe", "be"]
    expected_output_2_bytes = ["be41414141414141be", "be4141be", "bebe", "41"]
    expected_output_4_bytes = ["be41be414141be41be", "bebebebe", "4141", "41"]
    original_data = copy.copy(data)  # sic!
    last_str = None
    res = None
    while True:
        data, res = byteflip_1(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_1_byte[iteration_id]:
        print("byteflip1 failed, output is %s" % last_str)
    else:
        print("byteflip1 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None

    while True:
        data, res = byteflip_2(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_2_bytes[iteration_id]:
        print("byteflip2 failed, output is %s" % last_str)
    else:
        print("byteflip2 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None

    while True:
        data, res = byteflip_4(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_4_bytes[iteration_id]:
        print("byteflip4 failed, output is %s" % last_str)
    else:
        print("byteflip4 succeeded")
    return True


def test_arithmentic(data, iteration_id):
    res = None
    expected_output_1_byte = ["414141414141414141", "41414141", "4141", "41"]
    expected_output_2_bytes = ["404141414141414142", "40414142", "4042", "41"]  # TODO: check if it is correct output
    expected_output_4_bytes = ["414140414141414142", "41414042", "4141", "41"]  # TODO: check if it is correct output
    original_data = copy.copy(data)  # sic!
    last_str = None
    res = None
    while True:
        data, res = mutate_byte_arithmetic(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_1_byte[iteration_id]:
        print("arith1 failed, output is %s" % last_str)
    else:
        print("arith1 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None
    i = 0
    while True:
        data, res = mutate_2bytes_arithmetic(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break
        i += 1

    if not last_str or last_str != expected_output_2_bytes[iteration_id]:
        print("arith2 failed, output is %s" % last_str)
    else:
        print("arith2 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None

    while True:
        data, res = mutate_4bytes_arithmetic(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_4_bytes[iteration_id]:
        print("arith4 failed, output is %s" % last_str)
    else:
        print("arith4 succeeded")
    return True


# print ''.join('{:02x}'.format(x) for x in data)
def test_interesting(data, iteration_id):
    res = None
    expected_output_1_byte = ["7f7f7f7f7f7f7f7f7f", "7f7f7f7f", "7f7f", "7f"]
    expected_output_2_bytes = ["ffffffffffffffff7f", "ffffff7f", "ff7f", "41"]
    expected_output_4_bytes = ["ffffffffffffffff7f", "ffffff7f", "4141", "41"]
    original_data = copy.copy(data)  # sic!
    last_str = None
    res = None
    while True:
        data, res = mutate_1byte_interesting(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_1_byte[iteration_id]:
        print("interesting1 failed, output is %s" % last_str)
    else:
        print("interesting1 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None
    i = 0
    while True:
        data, res = mutate_2bytes_interesting(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break
        i += 1

    if not last_str or last_str != expected_output_2_bytes[iteration_id]:
        print("interesting2 failed, output is %s" % last_str)
    else:
        print("interesting2 succeeded")

    last_str = None
    data = copy.copy(original_data)
    res = None

    while True:
        data, res = mutate_4bytes_interesting(data, res)
        if not res:
            last_str = ''.join('{:02x}'.format(x) for x in data)
            break

    if not last_str or last_str != expected_output_4_bytes[iteration_id]:
        print("interesting4 failed, output is %s" % last_str)
    else:
        print("interesting4 succeeded")
    return True

tokens_list = [b"very_long_dict_string777777777777777", b"test", b"ext1", b"a"]
def test_dict(data, iteration_id):
    global tokens_list
    expected_output_overwrite = [b"AAAAAAAAA", b"Atest", b"AA", b"Aa"]
    expected_output_insert = [b'very_long_dict_string777777777777777AAAAAAAAA', b'AtestAAA', b"AAext1", b"Aa"]
    data_clean = copy.copy(data)

    # call AFL init to initialize dictionary
    AFLFuzzer(tokens_list, None, "test_file")

    data, func_state = dictionary_overwrite(data, [iteration_id, iteration_id])
    if data != expected_output_overwrite[iteration_id]:
        print("test_dict_overwrite failed, output is %s" % data)
    else:
        print("test_dict_overwrite succeeded")
    data, func_state = dictionary_insert(data_clean, [iteration_id, iteration_id])
    if data != expected_output_insert[iteration_id]:
        print("test_dict_insert failed, output is %s" % data)
    else:
        print("test_dict_insert succeeded")
    return True


def test_havoc(data, iteration_id):
    print("starting havocs")
    data = havoc_bitflip(data)
    print("Bitflip:", data)
    data = havoc_interesting_byte(data)
    print("Interesting byte:", data)
    data = havoc_interesting_2bytes(data)
    print("Interesting 2 bytes:", data)
    data = havoc_interesting_4bytes(data)
    print("Interesting 4 bytes:", data)
    data = havoc_randomly_add(data)
    print("Randomly add:", data)
    data = havoc_randomly_substract(data)
    print("Randomly subtract:", data)
    data = havoc_randomly_add_2bytes(data)
    print("Randomly add 2 bytes:", data)
    data = havoc_randomly_substract_2bytes(data)
    print("Randomly subtract 2 bytes:", data)
    data = havoc_randomly_add_4bytes(data)
    print("Randomly add 4 bytes:", data)
    data = havoc_randomly_substract_4bytes(data)
    print("Randomly subtract 4 bytes:", data)
    data = havoc_remove_randomly_block(data)
    print("Randomly remove block:", data)
    data = havoc_clone_randomly_block(data)
    print("Randomly clone block:", data)
    data = havoc_overwrite_randomly_block(data)
    print("Randomly overwrite block:", data)
    data = havoc_overwrite_with_dict(data)
    print("Randomly overwrite with dict:", data)
    data = havoc_insert_with_dict(data)
    print("Randomly insert with dict:", data)
    print("all havocs succeeded")
    return True


def test_splice(data, iteration_id):
    # merge with manul.config or unit_tests.py :)
    print("Starting splice")
    queue_path = "./"  # current path where we run this unit_tests.py
    list_of_files = [(1, "manul.config"), (1, "unit_tests.py")]
    data, func_state = splice(data, list_of_files, queue_path, None, 20)
    print("Result of splice:", data)
    return True


iteration = 0
def test_cycle(data):
    global iteration
    test_bitflip(copy.copy(data), iteration)
    test_byteflip(copy.copy(data), iteration)
    test_arithmentic(copy.copy(data), iteration)
    test_interesting(copy.copy(data), iteration)
    test_dict(copy.copy(data), iteration)
    test_havoc(copy.copy(data), iteration)
    test_splice(copy.copy(data), iteration)
    iteration += 1


def extra_test_havoc_remove_randomly_block():
    data_extra = b"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    for i in range(0, 500):
        data_extra = havoc_remove_randomly_block(data_extra)
        if data_extra == b"":
            print("extra_test_havoc_remove_randomly_block failed!")

def test_radamsa_library(): # TODO: make it work on Windows
    #test_cycle(bytearray("")) # no string at all
    radamsa_fuzzer = radamsa.RadamsaFuzzer(3)
    radamsa_fuzzer.load_library("./libradamsa/libradamsa.so")
    res = radamsa_fuzzer.radamsa_generate_output(b"ABDSDADA")
    print("Radamsa output %s" % res)
    if len(res) == 0:
        print("radamsa library test failed")

def extra_test_havoc_add_random_block():
    data = b'A'
    data = havoc_clone_randomly_block(data)
    if len(data) <= 1:
        print("extra_test_havoc_add_random_block failed!")
    print("Result of add random block %s" % data)

if __name__ == "__main__":
    test_cycle(bytearray("AAAAAAAAA", "utf-8"))  # regular string
    test_cycle(bytearray("AAAA", "utf-8"))  # short string
    test_cycle(bytearray("AA", "utf-8"))  # shorter string
    test_cycle(bytearray("A", "utf-8"))  # the shortest string
    extra_test_havoc_remove_randomly_block()
    extra_test_havoc_add_random_block()

    if is_bytearrays_equal(b"AAAAAA", b"AAAAAA") == False or is_bytearrays_equal(b"AAAAAAA", b"BEBEBEBE") == True:
        print("is_bytearray_equal failed")
    else:
        print("is_bytearray_equal succeeded")

    if sys.platform == "linux":
        test_radamsa_library()