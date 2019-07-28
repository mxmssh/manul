#   Manul - Windows utils
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

# Exception Types
# see https://github.com/Alexpux/mingw-w64/blob/master/mingw-w64-headers/include/ntstatus.h

STATUS_CONTROL_C_EXIT = 0xc000013a

#TODO: this is very broad description of error codes. Define it better.
EXCEPTION_FIRST_CRITICAL_CODE        = 0xC0000000   

EXCEPTION_LAST_CRITICAL_CODE         = 0xC03A0019