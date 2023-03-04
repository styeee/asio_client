#include <iostream>
using namespace std;
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <string>
#include <fstream>
#include <time.h>
#include <stdlib.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

ofstream logger;

#define divade "\n------------------------------------------------\n"
inline
void log(const string&text)
{
    auto t=time(0);
    const char*const time=asctime(localtime(&t));
    logger  <<divade<<time<<'['<<text<<']'<<divade;
    cout    <<divade<<time<<'['<<text<<']'<<divade;
}

///////////////////////////////////////////////////////////////////////

class client
{
    asio::ip::tcp::socket server;
    const string&expr;
    double answer;
public:
    client
    (
        asio::io_context&context,
        const string&expr,
        const asio::ip::tcp::endpoint&address
    )
        :server(context),expr(expr)
    {
        log("Try to connect with "+address.address().to_string());

        server.async_connect(address,
        [this](asio::error_code e)
        {
            if(e){log("Error on connection:"+e.message());exit(3);}

            log("Establish connection with server");
            log("Try to send expression "+this->expr+" on server");

            asio::async_write
            (
                server,
                asio::buffer(this->expr.c_str(),this->expr.size()+1),
                [this](asio::error_code e,size_t l)
                {
                    if(e){log("Error on send:"+e.message());exit(4);}

                    log("Sending success");
                    log("Wait for answer from server");

                    asio::async_read(server,asio::buffer(&answer,8),
                    [this](asio::error_code e,std::size_t)
                    {
                        if(e){log("Error on recieving answer:"+e.message());exit(5);}

                        log("Answer receiving success:"+to_string(answer));

                        cout<<"recieve:"<<answer<<endl;

                        server.close(e);
                        if(e){log("Error on closing connection:"+e.message());exit(6);}
                        log("Successful closing connection and finish job");
                    });
                }
            );
        });
    }
};

///////////////////////////////////////////////////////////////////////

int main(int argc,char*argv[])
{
    const char*ip="127.0.0.1";
    unsigned short port=1111;
    const char*log_path="client_log.txt";

    if(argc>1)ip=argv[1];
    if(argc>2)port=atoi(argv[2]);
    if(argc>3)log_path=argv[3];

    logger.open(log_path,ios_base::app|ios_base::end);
    if(!logger){cout<<"logger error";return 1;}
    log("Logger starting");

    cout<<"\nenter expression:";string expr;cin>>expr;
    log("Get expression from user: "+expr);
    
    try
    {
        asio::io_context context;
        client c
        (
            context,
            expr,
            asio::ip::tcp::endpoint(asio::ip::make_address(ip),port)
        );
        context.run();
    }
    catch(exception&e)
    {
        log("Exception: "+string(e.what()));
        return 2;
    }

    log("Logger stoping");
    logger.close();
    return 0;
}