*This project has been created as part of the 42 curriculum by aredouan.*

# Codexion

## Description
Codexion is a multithreaded resource-scheduling simulation inspired by the classic 42 synchronization projects. Each coder thread competes for two shared dongles in order to compile, then goes through debugging and refactoring phases before trying again.

The goal of the project is to coordinate many concurrent threads without deadlocks, starvation, or inconsistent logs. The implementation supports two scheduling modes, `fifo` and `edf`, and uses per-dongle waiting queues to decide which coder may proceed next.

## Instructions
### Compilation
```bash
make
```

### Cleaning
```bash
make clean
make fclean
make re
```

### Execution
```bash
./codexion nb_coders t_burnout t_compile t_debug t_refactor req_compiles cooldown fifo|edf
```

### Arguments
- `nb_coders`: number of coder threads and dongles.
- `t_burnout`: maximum time in milliseconds before a coder burns out.
- `t_compile`: time spent compiling while holding both dongles.
- `t_debug`: time spent debugging after compiling.
- `t_refactor`: time spent refactoring after debugging.
- `req_compiles`: number of compilations required before the simulation ends.
- `cooldown`: time in milliseconds before a released dongle becomes available again.
- `fifo|edf`: scheduler mode, either first-in-first-out or earliest-deadline-first.

## Blocking cases handled
- Deadlock prevention: dongles are acquired in a fixed address order, so coders never wait in a circular dependency.
- Coffman’s conditions: the implementation breaks circular wait and limits indefinite resource retention through ordered acquisition and explicit release.
- Starvation prevention: each dongle keeps a priority queue of waiting requests, so the next eligible coder is chosen deterministically instead of by chance.
- Cooldown handling: when a dongle is released, it is marked unavailable until `available_at`, which prevents immediate re-acquisition and keeps timing fair.
- Precise burnout detection: the monitor continuously compares `last_compile` against `t_burnout` and ends the simulation as soon as a coder exceeds the limit.
- Log serialization: all output goes through `write_mutex`, so status lines never interleave or corrupt each other.

## Thread synchronization mechanisms
- `pthread_mutex_t` protects shared simulation state such as `is_dead`, `sim_started`, `last_compile`, `compile_count`, and each dongle’s ownership fields.
- `pthread_cond_t` is used per dongle to wake sleeping coders when a resource is released or when the simulation ends.
- The custom event-like start barrier uses `sim_started` guarded by `death_mutex`, letting worker threads wait until the simulation time base is initialized.
- The monitor thread reads coder progress under the same protection as writers, which prevents races when checking burnout or completion.
- Coders and the monitor communicate through shared state plus wake-ups: coders update `last_compile` and `compile_count`, while the monitor sets `is_dead` and broadcasts to unblock all waiters.
- Example: a coder may sleep on a dongle’s condition variable, but a release broadcast wakes it only after the mutex-protected queue and availability state have been updated.

## Resources
- POSIX threads reference: `man pthread_mutex_lock`, `man pthread_cond_wait`, `man pthread_create`.
- GNU C Library manual: time functions and thread synchronization primitives.
- `gettimeofday(2)` documentation for millisecond timing.
- Classic 42 dining philosophers resources and concurrency notes for deadlock and starvation patterns.

### AI usage
AI was used as a sparring partner for advanced stress testing. Specifically, it assisted in generating edge-case scenarios (such as massive thread exhaustion, integer overflows, and helgrind data race analysis) to bulletproof the implementation. It also helped structure this README file to meet the project's documentation requirements