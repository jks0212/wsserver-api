#wsserver(WebSocket Server)

<p>c 기반의 웹 소켓서버 api입니다. 현재 간단한 통신 정도는 되지만 웹 소켓의 기능을 완벽하게 구현하기에는 아직 많은 개발이 필요합니다. wsserver api들은 웹 소켓 표준 문서 <a href="https://tools.ietf.org/html/rfc6455" target="_blank">RFC-6455</a>에 기반해서 만들었습니다.</p>


<p>wsserver에서는 sha1해시 알고리즘을 사용하기 위해 openssl 라이브러리를 사용하고 있습니다. 따라서 컴파일 시 gcc나 g++에 -lcrypto를 추가해서 컴파일해 주시기 바랍니다.
현재 wsserver는 비동기 통신은 지원되지 않으니 비동기 통신을 원하시는 분들은 참고해주세요.</p>


##Introduction to wsserver api

<pre><code style="padding: 10px">int wsaccept(int client_sock, char* recv_data);</code></pre>
<p>tcp계층에서 이미 연결이 된 클라이언트의 소켓과 웹 소켓 연결 헤더를 받아 연결을 해줍니다. 연결에 성공하면 0을 리턴하고 실패하면 -1을 리턴합니다.</P>


<pre><code>int wssend(int client_sock, const char* send_data, int length);</code></pre>
<p>웹 소켓 통신시 필요한 헤더를 추가해서 send해주는 함수입니다. 성공시 전송한 바이트 갯수를 실패시 -1을 리턴합니다.</p>

<pre><code>int wsrecv(int client_sock, char* recv_data, int length);</code></pre>
<p>클라이언트에서 보낸 데이터를 받으며 성공시 받은 바이트 갯수를 실패시 -1을 리턴합니다.</p>

<p>위 3가지 함수를 통해서 간단한 웹소켓 통신을 할 수 있으며 웹 소켓의 모든 기능을 구현하지 않았기 때문에 한 번 보낼 때 데이터를 125바이트 이하로 보내야 됩니다.</p>
















