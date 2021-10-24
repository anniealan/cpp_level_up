### Implement a multi-threaded message queue with the following interface:

```cpp
class MessageQueue
{ public:
   AddMessage(std::string);
   RegisterHandler(void (*handler)(std::string));
}
```

### Message handling must be organized asynchronously.