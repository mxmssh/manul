import os
import sys

'''
This is an example of third-party mutator plugin. Manul will load and call mutate for all files provided.
'''

def init():
    print("[PLUGIN] Init sucessfully completed")
    return

def mutate(data):
    data[0] += 1
    return data