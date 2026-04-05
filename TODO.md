## IMPROVEMENTS

- extract process information with ebpf
  - date
  - file
  - process Name
  - event Read, write, execute, delete, move
  - state of the process
  - umask
  - username: user name of the process
  - command line arguments
  - success: true or false
- Socket for email | log server reporting | uing known APIs for log submission
- Nested directories: 2-level
- Proper documentation
- Bug fix on json logging. [] gets written => [, {data}]
- Drop and maintain only necessary root CAPs

### Config format

[directories]
path ...
[files]
