# bhvtree [![CMake on multiple platforms](https://github.com/wwwvladislav/bhvtree/actions/workflows/test.yml/badge.svg)](https://github.com/wwwvladislav/bhvtree/actions/workflows/test.yml) ![MIT](https://img.shields.io/badge/license-MIT-blue.svg)
This library provides functionality for manipulating behavior trees. The library is written in C++17.
The main goal of the library is to be as small as possible and easier to integrate with other projects.
The library contains only two files located in the src subdirectory.
These files can simply be added to the project to take full advantage of its functionality.

# Supported Node Types
## Control nodes
Control nodes are a special type of node that can manage multiple children.
These types of nodes can execute a sequence of child nodes, executing them until one is successfully returned, and so on.
All control nodes have internal state.
This internal state is reset to the initial state after exception is thrown or execution completed.
Therefore, it must be safe for use with predicates and handlers that may throw an exception.

- **Sequence**

| Return  | Condition                    |
| ------- | ---------------------------- |
| success | If all children succeed      |
| failure | If one child fails           |
| running | If one child returns Running |

- **Fallback**

| Return  | Condition               |
| ------- | ----------------------- |
| success | If one child succeeds   |
| failure | If all children fail    |
| running | If one child returns Running |

- **Parallel**

| Return  | Condition               |
| ------- | ----------------------- |
| success | If at least M child succeed  |
| failure | If at least N - M child fail |
| running | In other cases               |

- **If/Then/Else**

An if-else statement controls conditional branching.
This is similar to the if/then/else expressions used in programming languages.

| Return  | Condition               |
| ------- | ----------------------- |
| success | If the branch handler succeed             |
| failure | If the branch handler fails or is missing |
| running | In other cases                            |

- **Switch/Case/Default**

An switch-case statement is another way to control the conditional branching.
It is similar to the parallel control node with a slight difference.
Instead of executing all nodes, nodes are selected by predicate.
All handlers with successfully completed predicates will be executed.

| Return  | Condition               |
| ------- | ----------------------- |
| success | If all case handlers are succeed                              |
| failure | If least one case handler is failed                           |
| running | In other cases, i.e least one predicate or handler is running |

## Decorators
Decorators are control nodes with single child. These node types modify the behavior of the controlled child node.
For instance, the result of a child node may be inverted, or a controlled node may be re-executed multiple times.
The full list of supported decorators will be described in the sections below.

- **Invert**

| Return  | Condition               |
| ------- | ----------------------- |
| success | If child fail           |
| failure | If child succeed        |
| running | If child running        |

- **Repeat**

Repetition of the child node N times. If the child node completed with an error, an error status is also returned.
i.e, all executions of the child node must be successful to return a successful status.

| Return  | Condition                |
| ------- | ------------------------ |
| success | If child succeed N times |
| failure | If child fail            |
| running | If child running         |

- **Retry**

Attempt to execute a child node until it completes successfully.
The number of attempts can be limited or unlimited and is configurable via the node constructor.

| Return  | Condition                |
| ------- | ------------------------ |
| success | If child succeed within N attempts |
| failure | If child failed N times            |
| running | If child running                   |

- **Force success/failure**

This node type ignores the success/failure of the child node and always returns the specified state or execution state.

| Return  | Condition                |
| ------- | ------------------------ |
| success | If the child succeed or failed and the node is configured to return success state |
| failure | If the child succeed or failed and the node is configured to return failed state  |
| running | If child running                   |

## Execution nodes
Execution nodes are leaf nodes with a specific logic or function. These leaf nodes are usually declared as user-defined lambda functions.

- **Action**
- **Condition**
