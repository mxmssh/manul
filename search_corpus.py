  
#   Search corpus is a script to automatically search and pull
#   corpus of certain type from GitHub.
#   -------------------------------------
#   Maksim Shudrak <mxmssh@gmail.com>
#
#   Copyright 2020 Salesforce.com, inc. All rights reserved.
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

import os
import sys
from github import Github
from git import Repo
import random
import string
import uuid
import subprocess
import shutil
import hashlib


def hash_file(file_full_path):
    # BUF_SIZE is totally arbitrary, change for your app!
    BUF_SIZE = 65536  # lets read stuff in 64kb chunks!
    sha1 = hashlib.sha1()
    with open(file_full_path, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)

    return sha1.hexdigest()


def correct_file_format(file_path, file_format):
    res = subprocess.check_output(["file", file_path])
    result_string = res.decode('utf-8')
    actual_type = result_string[result_string.find(":"):]
    return file_format.lower() in actual_type.lower()

def get_target_files(new_repo_path, file_format):
    interesting_files = list()
    for root, directories, filenames in os.walk(new_repo_path):
        for filename in filenames:
            full_file_path = os.path.join(root, filename)
            if (correct_file_format(full_file_path, file_format)):
                print("Found matching file format for %s: %s" % (file_format, full_file_path))
                interesting_files.append(full_file_path)
    return interesting_files

def copy_files(target_files, target_dir):
    for target_file in target_files:
        new_name = hash_file(target_file)
        print("Copying %s as %s in %s" % (target_file, new_name, target_dir))
        shutil.copy(target_file, target_dir + "/" + new_name)
    return 0

github_url = "https://github.com/"
g = Github("<YOUR_GITHUB_API_KEY_HERE>")
#TODO create dir
target_dir = sys.argv[2]
file_type = sys.argv[3]

repositories = g.search_repositories(query=sys.argv[1])
for i, repo in enumerate(repositories):
    print("Cloning %d %s" % (i, repo))
    git_url = github_url + repo.full_name
    uuid_str = uuid.uuid4()
    new_repo_path = "repos/" + str(uuid_str)+"_"+repo.name
    try:
        Repo.clone_from(git_url, new_repo_path, depth=1)
    except:
        print("Failed to clone repo")
        continue
    target_files = get_target_files(new_repo_path, file_type)
    copy_files(target_files, target_dir)
