<!DOCTYPE html PUBLIC "-//w3c//dtd html 4.0 transitional//en">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html;
      charset=windows-1252">
    <meta name="Generator" content="Microsoft Word 97">
    <meta name="GENERATOR" content="Mozilla/4.76 [en] (WinNT; U)
      [Netscape]">
    <title>Um Sistema Genérico de Compartilhamento de Arquivos
      (SNAPSTER)</title>
  </head>
  <body>
    <center><font face="Arial,Helvetica,sans-serif" size="2">
        <h2>REDES DE COMPUTADORES A<br>
          <br>
          Projeto 2 - 1º Semestre 2016<br>
          <br>
          Sistema de envio de mensagens e fotos peer-to-peer (WhatsAp2p)</h2>
        <h2><font face="Arial,Helvetica,sans-serif" size="2"> </font></h2>
      </font><br>
    </center>
    <h2><font face="Arial,Helvetica,sans-serif" size="2"><font
          face="Arial,Helvetica,sans-serif" size="2"><font
            face="Arial,Helvetica,sans-serif" size="2">
            <h2><b>Desenvolvimento do Projeto</b></h2>
          </font></font></font></h2>
    <font face="Arial,Helvetica,sans-serif" size="2">
      <p><font face="Arial,Helvetica,sans-serif" size="2"><font
            face="Arial,Helvetica,sans-serif" size="2"><span
              style="font-family: Helvetica,Arial,sans-serif;">O projeto
              deverá ser desenvolvido em <span style="font-weight:
                bold;">grupos de no máximo <font
                  face="Arial,Helvetica,sans-serif">5</font> integrantes</span>.<br>
            </span></font></font></p>
    </font> <font face="Arial,Helvetica,sans-serif" size="2"> <font
        face="Arial,Helvetica,sans-serif" size="2"><font
          face="Arial,Helvetica,sans-serif" size="2"><big> </big>
          <h2><b>Normas de conduta<br>
            </b></h2>
        </font></font>
      <p><font face="Arial,Helvetica,sans-serif" size="2"><font
            face="Arial,Helvetica,sans-serif" size="2"> <span
              style="font-family: Helvetica,Arial,sans-serif;">A
              interação entre os grupos é estimulada, no entanto
              qualquer tentativa de plágio de trabalhos será punida com
              a </span><span style="font-weight: bold; font-family:
              Helvetica,Arial,sans-serif;">nota -Nmax nos trabalhos para
              todos os envolvidos</span><span style="font-family:
              Helvetica,Arial,sans-serif;">.<br>
            </span></font></font></p>
    </font>
    <p style="font-family: Helvetica,Arial,sans-serif;"><b><font
          size="+1">Introdução</font></b> </p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>O
        objetivo deste projeto é implementar um sistema de envio de
        mensagens e fotos peer-to-peer (WhatsAp2p) em linguagem C.<br>
      </small></p>
    <b style="font-family: Helvetica,Arial,sans-serif;"><font size="+1">Arquitetura



        do Sistema e Implementação</font></b>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>O sistema
        consistirá de um <b>servidor central</b> e </small><small>de
        um <b>módulo de usuário</b>. &nbsp; </small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>O <b>servidor



          central </b>deverá permitir aos usuários localizar os demais
        usuários online. Cada usuário é identificado no servidor central
        por um <u>número de telefone</u> e sua localização correponde
        ao <u>endereço IP</u> e à <u>porta</u> onde o <b>módulo de
          usuário</b> deste usuário aguardará por mensagens e fotos. <br>
      </small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>O <b>módulo

          de usuário</b> deverá permitir ao usuário enviar mensagens e
        fotos a outros usuários e também receber mensagens e fotos de
        outros usuários, a partir das informações de localização obtidas
        do <b>servidor central</b>.</small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>Quando um
        <b>módulo de usuário</b> inicia a sua execução, ele deve
        solicitar</small><small> ao usuário que informe o seu <u>número
          de telefone</u> e deve enviar essa informação ao <b>servidor
          central</b>, junto com o <u>endereço IP</u> e a <u>porta</u>
        onde aguardará por mensagens e fotos. O <b>servidor central</b>
        deve então armazenar os dados de identificação e localização do
        usuário, registrando este usuário como online. Usuários não
        registrados no servidor central serão identificados como offline
        e portanto não serão capazes de receber mensagens e fotos. </small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>O usuário
        deve ser capaz de adicionar um contato e também de criar grupos
        de contatos através de seu <b>módulo de usuário</b>. As
        informações dos contatos e grupos de um usuário devem ser
        armazenadas em um <u>arquivo local</u>, de forma que ao iniciar
        o <b>módulo de usuário</b> essas informações sejam
        automaticamente carregadas. Ao solicitar o envio de uma mensagem
        ou foto a um contato ou a um grupo, o <b>módulo de usuário</b>
        deve realizar uma consulta ao <b>servidor central</b> para
        obter a localização desses contatos e em seguida deve enviar a
        mensagem ou foto diretamente ao <b>módulo de usuário</b> deste
        usuário, sem passar pelo <b>servidor central</b>. </small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><small>Quando um
        usuário finalizar o seu <b>módulo de usuário</b>, toda a
        informação relativa à localização deste usuário deve ser
        removida do <b>servidor central</b>. </small></p>
    <p style="font-family: Helvetica,Arial,sans-serif;"><b><font
          size="+1">Observações Gerais</font></b> </p>
    <ol style="font-family: Helvetica,Arial,sans-serif;">
      <small> </small>
      <li><small>O <b>servidor central </b>e os <b>módulos de usuário</b>
          devem suportar acesso concorrente.</small></li>
      <small> </small> <small> </small>
      <li><small>Deve ser possível executar mais de um <b>módulo de
            usuário</b> no mesmo computador.<br>
        </small></li>
      <li><small>O endereço IP&nbsp; e a porta do <b>servidor central</b>
          deverão ser passados como parâmetro para o <b>módulo de
            usuário</b> na linha de comando.</small></li>
    </ol>
    <p><font style="font-family: Helvetica,Arial,sans-serif;" size="2"><font
          size="2"><span style="font-weight: bold;">Data de entrega:</span>
          07 de Junho</font></font> </p>
    <font style="font-family: Helvetica,Arial,sans-serif;" size="2"><font
        size="2">Deve ser entregue um <span style="font-weight: bold;">relatório

          impresso</span> contendo: </font></font><font
      style="font-family: Helvetica,Arial,sans-serif;"
      face="Arial,Helvetica,sans-serif" size="2"><big> </big><font
        style="font-family: Helvetica,Arial,sans-serif;" size="2"><font
          size="2"> <big><big> </big></big>
          <li>detalhes do projeto da aplicação, apresentando uma
            descrição de alto nível dos programas implementados
            (contendo textos explicativos e diagramas) e o formato das
            requisições e respostas enviadas<br>
          </li>
          <big><big> </big><big> </big></big>
          <li>detalhes de implementação relevantes (como por exemplo,
            como é feito uso de threads ou processos filhos) <br>
          </li>
          <big><big> </big><big> </big></big>
          <li>descrição do processo de compilação (<font size="2"><font
                size="2">contendo screenshots que mostrem os comando
                utilizados na compilação e os resultados obtidos) </font></font></li>
          <big><big> </big><big> </big></big>
          <li>descrição dos testes realizados (<font size="2"><font
                size="2">contendo screenshots da execução dos programas
                e os resultados obtidos, </font></font><font size="2"><font
                size="2">demonstrando que o programa realiza as tarefas
                solicitadas</font></font><font size="2"><font size="2">)
              </font></font><br>
            <font size="2"> </font> </li>
          <big><big> </big><big> </big></big>
          <li>os códigos fonte dos programas implementados</li>
          <big><big> </big></big> </font></font><br>
      <font style="font-family: Helvetica,Arial,sans-serif;" size="2"><font
          size="2"><font face="Arial,Helvetica,sans-serif">C</font>ada
          integrante do grupo deverá postar em seu escaninho no AVA o <span
            style="font-weight: bold;">relatório em formato digital
            (.pdf)</span> e o <span style="font-weight: bold;">código
            fonte de todos os programas implementados</span>.</font></font></font><font
      face="Arial,Helvetica,sans-serif" size="2"><span
        style="font-family: Helvetica,Arial,sans-serif;"></span></font>
  </body>
</html>
