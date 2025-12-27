# Core Invariants — cpp-proxy

This document defines the non-negotiable rules of the system.
If any future design or code violates these rules, the code is wrong.

---

## 1. File Descriptor Ownership

- Every file descriptor (FD) has exactly **one owner** at any time.
- Ownership is explicit and enforced in code.
- No FD is shared implicitly.
- No FD is closed twice.
- No FD is leaked.

---

## 2. No Blocking Syscalls

- No blocking syscall is allowed anywhere in the system.
- All sockets must be non-blocking.
- Any operation that may block must be guarded by readiness notification (epoll).
- Blocking is considered a bug, not an optimization issue.

---

## 3. epoll Does Not Do Work

- epoll only reports readiness:
  - readable
  - writable
  - error / hangup
- epoll never performs:
  - reads
  - writes
  - protocol parsing
  - state transitions
- epoll is a notification mechanism, not an execution engine.

---

## 4. Reads and Writes Are Partial

- A single read() may return:
  - fewer bytes than requested
  - zero bytes
- A single write() may write:
  - fewer bytes than provided
- Code must never assume full reads or full writes.
- Buffers must track offsets explicitly.

---

## 5. Explicit Connection State Transitions

- Every connection has a clearly defined state.
- State transitions are explicit and intentional.
- No implicit state changes based on side effects.
- Invalid transitions are logic bugs.

---

## 6. No Component Calls exit()

- No library or internal component may terminate the process.
- Errors must be propagated upward.
- The application layer decides if and when to exit.

---

## 7. Deterministic Cleanup

- All resources are released deterministically.
- Cleanup happens via RAII, not garbage collection.
- Destructors must be safe to call at any time.
- Cleanup must not depend on epoll events or callbacks.

---

## 8. Correctness Over Convenience

- Simplicity is preferred over cleverness.
- Explicit code is preferred over implicit behavior.
- Performance optimizations come only after correctness.

---

This document is the ground truth.
When in doubt, follow these invariants.
