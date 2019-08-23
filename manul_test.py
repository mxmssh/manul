import os
import sys
import subprocess

print("Starting Manul")
if sys.platform == "win32":
    python_path = os.getenv("PYTHONENV")
    os.system("%s/python.exe manul.py -i in -o out --stop_after_nseconds 7 -s \"win/test/test64.exe @@\"" % python_path)
    sys.exit(0)
else:
    os.system("python manul.py -i in -o out --stop_after_nseconds 7 \"linux/test/test_afl @@\"")
    print("Checking if files exist")
    path, dirs, files = next(os.walk("./out/0/crashes/"))
    file_count = len(files)
    print("Found %d crashes" % file_count)

    if file_count >= 1:
        print("Success")
        sys.exit(0)
    else:
        print("Fail")
        sys.exit(1)

