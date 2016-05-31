<H1 align ="center">WhatsAP2P</H1>
<H3 align ="center">Aplicação peer-to-peer de troca de mensagens e fotos.</H3>
 




Este projeto consiste em uma aplicação p2p híbrida de troca de mensagens.

Para se conectar à rede, o cliente deve efetuar "login" e informar ao servidor em qual endereço IP e porta ele estará esperando requisições. 

Para enviar uma mensagem (ou foto), o usuário deverá enviar uma requisição ao servidor contendo o número de celular do destinatário ao que deseja enviar a mensagem. Então o servidor irá retornar ao usuário o endereço IP e porta no qual o destinatário estará esperando as requisições. A partir deste momento, o remetente saberá as informações para contatar o destinatário, e uma vez que o destinatário recebê-las, ele também saberá as informações do rementente, possibilitando assim a conexão p2p.

O mesmo deverá ocorrer com mensagens enviadas em broadcast (para múltiplos destinatários), porém cada destinatário não terá informações sobre os outros destinatários.
