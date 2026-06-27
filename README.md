# A Linux File System Monitoring Daemon

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg) ![License](https://img.shields.io/badge/license-MIT-green) ![Status](https://img.shields.io/badge/status-active-brightgreen)

It is a lightweight file system monitoring daemon built for Linux using the fanotify API. It tracks file access and modification events within specified directories, providing users with detailed logs of filesystem activity in real time.

## Features

Monitors file access and modification events in real time using fanotify.
Supports logging of basic file events (read, write) with timestamps.
Gracefully handles signals like SIGHUP (reload configuration) and SIGTERM (terminate daemon).
Simple configuration file to add watch directories/files via a `.config/cf.conf` file in the root directory of the project.

## Upcoming Features

Future releases aim to add:

Support for nested directory monitoring.

- Enhanced logging with user-defined log formats and event types.
- Config file monitoring and reloading on the fly.

## Installation

Clone the repository:

```bash
git clone https://github.com/c0d-0x/h4shfsmon.git
cd h4shfsmon
mkdir bin
mkdir .config ## for configuration
make
```

### Run the daemon with required permissions (root access required for fanotify):

- Usage: add a dir or a file to be watch in `h4shfsmon/.config/cf.conf` file

```bash
sudo ./h4shfsmon -d # to run in debug, interactive mode
```

The daemon will log all file access and modification events to the console or log file (if specified).

## Command-Line Options

-d : Run as a daemon in the background.<br>
-v : Verbose mode for additional logging details.<br>
Example:

```bash
sudo ./bin/h4shfsmon -d
```

## Configuration

This is done in the `./.config/cf.conf`

## Logging

h4shfsmon logs events such as file accesses and modifications. The log output includes:

- Timestamp: When the event occurred.
- Event Type: File read or write - [ACCESS or MODIFIED].
- File Path: Full path of the affected file.
- Process Info: PID and name of the process accessing the file.

```bash
#| timestamp | event | process |umask | status | user | path|
27/06/2026 19:08:55 FILE_ACCESSED cat 0022 (running) c0d_0x /home/c0d_0x/secret.txt
```

## Signals

- SIGTERM: Gracefully shuts down the daemon.

## How it Works

- **Loads configuration:** Reads the configuration file and builds a list of paths to monitor.
- **Creates fanotify instance:** Initializes a fanotify file descriptor with appropriate flags.
- **Adds watches:** Adds watches for each configured path using fanotify_mark.
- **Event loop:** Continuously reads events from the fanotify file descriptor.
- **Event handling:** Processes events, retrieves process information, and optionally logs events.

## Dependencies

- Linux kernel with fanotify support
- poll library
- Standard C libraries

## Performance

It is optimized for lightweight monitoring of directories and mount points. For larger systems with many files, the following performance improvements are planned:

- Recursive directory monitoring to track nested directories.
- Batch event processing to reduce resource consumption under high load.
- Contributions

## Contributions are welcome! If you’d like to help improve h4shfsmon, consider working on the following areas:

- Adding recursive directory monitoring.
- Implementing support for different file event types (creation, deletion, permission changes).
- Optimizing resource usage through epoll or multi-threading.
- Enhancing logging to support custom formats and integration with syslog or SIEM systems.
- Feel free to fork the repository, submit issues, or create pull requests.

## Future Work

Here’s a glimpse of upcoming features and enhancements:

- Nested directory monitoring.
- Configurable logging output.
- Memory and resource optimization.
- Hash based file integrety check
- Web-based dashboard for real-time monitoring.

### NB: This project is currently under development.
