### Implement a class which can be shared between processes with the following interface:

```cpp
class Shared {
public:
std::string GetMessage();
void SetMessage();
void RegisterCallback(void (*f) (std::string) );
};
```
### Callback functions must be called in all the interedted processes if the message gets set in any other process.