# Селезнев Григорий Ильич ИДЗ-4 вариант 5
## Условие:
Задача об обедающих философах. Пять философов сидят возле круглого стола. Они проводят жизнь, чередуя приемы пищи и
размышления. В центре стола находится большое блюдо спагетти. Спагетти длинные и запутанные, философам тяжело управляться с ними, поэтому каждый из них, что бы съесть порцию, должен пользоваться двумя вилками. К несчастью, философам дали только пять вилок. Между каждой парой философов лежит одна вилка, поэтому эти высококультурные и предельно вежливые люди договорились, что каждый будет пользоваться только теми вилками, которые лежат рядом с ним (слева и справа). Написать программу, моделирующую поведение философов Программа должна избегать фатальной ситуации, в которой все философы голодны, но ни один из них не может взять обе вилки (например, каждый из философов держит по одной вилки и не хочет отдавать ее). Решение должно быть симметричным, то есть все процессы–философы должны выполнять один и тот же код. Философы являются отдельными клиентами, синхронизируемыми посредством сервера.

## Отчет на 4-5

### Сценарий задачи
1. Изначально запускается сервер [server.cpp -> server]
2. Далее сервер ждет подключение 5 философов (клиентов) [client.cpp -> client]
3. После подключения всех 5 философов начинается обед, философы отправляют сообщение о взятие вилки, сервер отправляет подтверждение, что философ взял две вилки, или отказ, что философу не удалось взять две вилки
4. Каждый философ должен покушать 3 раза, после чего завершает свою трапезу
5. Сервер останавливается после того, как все 5 философов пообедали 3 раза

### Запуск программы
1. Чтобы запустить сервер нужно прописать в консоле:
```
g++ -std=c++20 server.cpp -o server
./server <ip> <port>
```
2. Чтобы запустить клиентов нужно прописать в консоле:
```
g++ -std=c++20 client.cpp -o client
./client <id> <ip> <port>
```
<id> - от 0 до 4;

[Исходный код]()  
[Демонстрация программы]()


## Отчет на 6-7
