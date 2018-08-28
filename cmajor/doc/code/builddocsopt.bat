@echo off
cmdoc --verbose --optimize --build-threads=1 cmdoc/code.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 compression/cmdoc/compression.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 http/cmdoc/httpClients.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 json/cmdoc/json.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 solution/cmdoc/solution.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 socket/cmdoc/socketClients.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 threading/cmdoc/threading.cmdoc.xml
cmdoc --verbose --optimize --build-threads=1 xml/cmdoc/xml.cmdoc.xml
