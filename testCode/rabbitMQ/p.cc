#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>

int main()
{
    // 实例化底层网络通信框架的IO事件监控句柄
    auto *loop = EV_DEFAULT;

    // 实例化libEVHandler句柄（将AMQP框架与事件监控关联）
    AMQP::LibEvHandler handler(loop);

    // 实例化网络连接对象
    AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://root:123456@localhost:5672/"));

    // 实例化信道对象
    AMQP::TcpChannel channel(&connection);

    // 声明交换机并设置声明成功与失败的回调函数
    channel.declareExchange("test-exchange", AMQP::ExchangeType::direct)
            .onError([](const char *message){ std::cout << "声明交换机失败 : "<< message << "\n"; exit(0); })
            .onSuccess([](){ std::cout << "声明交换机成功" << "\n"; }); 

    // 声明队列并设置声明成功与失败的回调函数
    channel.declareQueue("test-queue")
            .onError([](const char *message) { std::cout << "声明队列失败 : "<< message << "\n"; exit(0); })
            .onSuccess([](){ std::cout << "声明队列成功\n"; });

    // 绑定交换机和队列
    channel.bindQueue("test-exchange", "test-queue", "test-queue-key")
            .onError([](const char *message) { std::cout << "绑定交换机和队列失败 : "<< message << "\n"; exit(0); })
            .onSuccess([]() { std::cout << "绑定交换机和队列成功\n"; });

    // 向交换机发布消息
    for (int i = 0; i < 10; ++i)
    {
        std::string msg = "hello" + std::to_string(i);
        if (!channel.publish("test-exchange", "test-queue-key", msg))
        {
            std::cout << "消息发布失败\n";
        }
    }

    // 启动底层网络通信框架，开启IO
    ev_run(loop, 0);

    return 0;
}