# vsclib

C utility library.

## Goals

* Provide useful functionality the C standard library lacks.
* Strive to be compatible with linear allocation strategies where possible.
* Have no dependencies other than the C standard library.
* Avoid or minimise use of floating-point arithmetic.

## Notes

* I mainly use this on Linux, Windows support is not maintained. Patches are welcome.

## License
This project is licensed under the [Apache License, Version 2.0](https://opensource.org/licenses/Apache-2.0):

Copyright &copy; 2020 [Zane van Iperen](mailto:zane@zanevaniperen.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## 3rd-party Licenses


| Project                                                | License              | License URL                                         |
|--------------------------------------------------------|----------------------|-----------------------------------------------------|
| [getline/getdelim](https://github.com/ivanrad/getline) | Public Domain        | https://github.com/ivanrad/getline                  |
| [xxHash](https://github.com/Cyan4973/xxHash)           | BSD 2-Clause License | https://www.opensource.org/licenses/bsd-license.php |
| [musl](https://musl.libc.org/)                         | MIT                  | https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT  |
