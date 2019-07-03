from afl_fuzz import *
import copy

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
        print("bitflip1 successed")

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
        print("bitflip2 successed")

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
        print("bitflip4 successed")

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
        print("byteflip1 successed")

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
        print("byteflip2 successed")

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
        print("byteflip4 successed")
    return True


def test_arithmentic(data, iteration_id):
    res = None
    expected_output_1_byte = ["414141414141414141", "41414141", "4141", "41"]
    expected_output_2_bytes = ["404141414141414142", "40414142", "4042", "41"] # TODO: check if it is correct output
    expected_output_4_bytes = ["414140414141414142", "41414042", "4141", "41"] # TODO: check if it is correct output
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
        print("arith1 successed")

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
        print("arith2 successed")

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
        print("arith4 successed")
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
        print("interesting1 successed")

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
        print("interesting2 successed")

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
        print("interesting4 successed")
    return True


def test_dict(data, iteration_id):
    #todo finish it
    return True


def test_havoc(data, iteration_id):
    #todo finish it
    return True


def test_splice(data, iteration_id):
    #todo finish it
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

if __name__ == "__main__":
    test_cycle(bytearray("AAAAAAAAA", "utf-8")) # regular string
    test_cycle(bytearray("AAAA", "utf-8")) # short string
    test_cycle(bytearray("AA", "utf-8")) # shorter string
    test_cycle(bytearray("A", "utf-8")) # the shortest string
    #test_cycle(bytearray("")) # no string at all
