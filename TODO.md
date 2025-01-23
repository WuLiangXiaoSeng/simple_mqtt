1. message type 的检查不合理，应该只检查前高4bit，低4比特不同时返回不同的错误

### CONNECT Message
 - [ ] Server端Client ID 长度支持0及大于23字节
 - [ ] Server端协议名称支持除 "MQTT" 以外的字符