# O que mudou em relação ao original

Projeto original: [TOTKDroid](https://github.com/mamcjr/TOTKDroid), de mamcjr. Todas as versões, da b1 à b7, partem da
`0.2.1-beta`. A versão de cada etapa está no histórico do repositório (ramo `s23`).

O motor é o mesmo do BOTWRecompAndroid. Muito do que está aqui veio da versão S23 do BOTW
([BOTWRecompAndroid-S23](https://github.com/Moab-tk/BOTWRecompAndroid-S23)), conferido trecho por trecho.

## Resumo por área

| Área | O que mudou |
|---|---|
| Pacote | `com.totkdroid.s23`, instala ao lado do original; nome do app "TOTK S23" |
| Compilação | Feita para o processador do Snapdragon 8 Gen 2 |
| Ritmo dos quadros | Tela em 60 Hz; a partir da b6, quadros presos ao ritmo real dela (quadros regulares a 30 FPS) |
| Processador | Orçamento de 30 FPS avisado ao Android (ADPF) |
| Shaders | Quem espera faz a tarefa, tabelas maiores, registro das esperas longas, montagem antecipada |
| Drivers | Cinco drivers Turnip dentro do app, com licenças; o recomendado escolhido pelo celular |
| Configurações | Recomendadas com um toque, 8 blocos salvos, gravação segura |
| Interface | Maior, em cartões, tamanho do texto ajustável, textos em português |
| Calor | Controle de temperatura opcional: escala em tamanhos inteiros (b5); decide pelo aviso do Android ou pela temperatura do chip (b6) |
| Toque | Sensibilidade do arrasto da câmera até 800% |
| Testes | Chaves `LARECOMP_*` só com `testes.txt`, que só o `adb` cria |

## Versão por versão

### b1: base para o S23 (02/11/2026)

- Traz da versão S23 do BOTW: ritmo de quadros, som com novas tentativas, shaders, registro de falhas em arquivo,
  blocos de configurações, drivers no app, configurações recomendadas, opções de teste na aba Vídeo e interface.
- Pacote próprio, licenças dos drivers, testes protegidos pelo `testes.txt`, telas ajustadas. O visual Zonai do TOTK
  foi mantido. A seção de DLC do BOTW saiu. FIFO continua o padrão do TOTK.
- Medido na parte guiada: 25,5 FPS com a GPU limitada pelo sistema e 26,8 com ela liberada.

### b2: orçamento de 30 FPS

- O app avisa ao Android que o jogo roda a 30 FPS, para ele não baixar o processador além da conta.
- Configurações recomendadas iguais às usadas no S23 Ultra.

### b3: controle de temperatura (opcional)

- Em Vídeo, desligado por padrão: reduz a cena por etapas de 10% antes de o sistema cortar a GPU, pela folga térmica que
  o Android informa, e volta a subir depois de 60 s de calma. `LARECOMP_LOG_HEAT=1` registra o calor a cada 30 s.

### b4: revisão e otimizações do BOTW b10

- Revisão: esperas no relógio monotônico, conferência de tipo e endereço no kernel, gravação segura das configurações
  (cópia, troca e trava), programa de shader e sincronia esperam a linha de desenho, espera da GPU com mais 10 s,
  listas de comandos acima de 64, tabela de compute de 2048, anisotrópico no limite do driver, ADPF desiste depois de 10
  recusas, áudio e escrita de arquivo conferidos, tela de bloqueio só com `testes.txt`, nomes perigosos ignorados ao
  copiar pastas e 43 textos que apareciam em inglês.
- Montagem antecipada de pipelines (`cache/known_pipelines.bin`), numa fila de baixa prioridade. A primeira sessão
  depois de instalar só anota. `LARECOMP_NO_PREWARM=1` desliga.
- Cache de pipelines gravado numa linha própria; limpeza das vistas de textura por contagem; menu com mais lugares de
  textura.
- Medido em 30 minutos jogando (1x, saída 1080, calor desligado): 27,7 FPS de média, 71% das medições em 27 ou mais,
  29,8 dentro de um santuário. As quedas para 17 a 24 FPS vêm de áreas com 4 a 5 mil desenhos por quadro, onde o
  processador é o limite, e pioraram quando o sistema baixou o processador pelo calor.

### b5: cacos do controle de temperatura e sensibilidade (05/11/2026)

- **Cacos nas bordas, quadradinhos pretos e o sol estourado** com o controle de temperatura ligado (vistos no S23 na b4;
  sumiram com ele desligado). A cena do TOTK é 1600x896 e as imagens menores do jogo (metade, um quarto, um oitavo)
  são reduzidas uma por uma. Com o controle em 90% da opção 75%, a cena ficava com 605 linhas e o seu quarto com 151,
  contra 151,25 da conta certa. As passadas que leem a imagem menor nas bordas da maior liam fora do lugar.
- A escala da cena agora é sempre múltiplo de 1/8, que deixa 1600x896 e todas as metades com tamanho inteiro: 90% vira
  87,5%, 80% vira 75%, 67,5% vira 62,5%. As escalas 75%, 50% e as inteiras não mudam. `LARECOMP_SCALE_SNAP=0` volta ao
  jeito antigo.
- Sensibilidade do arrasto da câmera até 800% (era 300%), com uma dica: acima de uns 400% o direcional virtual já fica
  todo inclinado, e a câmera não gira mais rápido que o máximo do próprio jogo.
- Não foi publicada sozinha: tudo o que ela tem entrou na b6.

### b6: consertos da análise do código e auditoria das configurações (05/11/2026)

Feita a partir de uma análise do código inteiro do TOTK (14 achados, nenhum que fechasse o jogo) e de uma conferência
de todas as 45 configurações: se cada uma chega ao jogo e quando passa a valer.

- **Quadros regulares a 30 FPS de verdade:** no TOTK a opção só pedia a tela em 60 Hz; a parte que segura cada quadro no
  ritmo da tela só existia para o BOTW. Agora o TOTK também tem a grade dos quadros. Só a 30 FPS com a opção ligada;
  desligada, tudo como antes. `LARECOMP_TOTK_GRID=0` desliga só isso. **Ainda não medido no S23.**
- **Controle de temperatura:** escolha de como decide: pelo aviso do Android (como antes) ou pela temperatura do chip,
  com o limite de 75 a 95 °C (85 por padrão). Num celular com o limitador térmico do sistema desligado, o aviso do
  Android vinha com o chip a 70-76 °C e cortava a imagem sem precisar.
- **Configurações recomendadas** desligam o controle de temperatura (antes ele ficava como estava).
- **GPU sempre acordada** (opção de teste) volta a desligar quando o app vai para o segundo plano: a função nativa tinha
  o nome do BOTW e o Java do TOTK não a achava.
- Textos: proporção da tela (o Livre já enche a tela larga no TOTK, e vale ao começar o jogo de novo), versões (só a
  1.4.3), créditos do TOTK, saves com os nomes do TOTK, arquivo de registro e de falhas com o nome do TOTK
  (`totkrecomp-crash.txt`), "Bateria" e "conectado" em português, a cena contada como 1600x896.
- Robustez: arquivo de configurações salvo no Windows (fim de linha) funciona; memória física só dentro da janela do
  jogo; a lista de pipelines conhecidos é gravada sem segurar a trava da linha de desenho.
- **Movimento pelos analógicos** (pedido do Moab, para os quebra-cabeças de movimento dos santuários): no menu do app,
  aba Sistema > Controles. O analógico esquerdo inclina o controle até 60° para qualquer lado e volta reto ao soltar; o
  direito gira mais e deixa onde parou, para virar o quebra-cabeça de cabeça para baixo. Enquanto ligado, os analógicos
  não mexem o personagem nem a câmera, o sensor do celular fica de lado e um aviso no alto da tela mostra que está
  ligado. Não fica salvo: o app sempre abre com ele desligado.
- **Botão rápido do movimento pelos analógicos:** opção "Botão rápido na tela", logo abaixo (fica salva, vem desligada).
  Aparece um botão no canto de cima à esquerda: um toque liga ou desliga o movimento pelos analógicos sem abrir o menu
  (ele acende quando está ligado). Segurando e arrastando, ele vai para qualquer lugar da tela, como o painel de
  desempenho, e fica onde for deixado. O toque nele não vai para o jogo.
- Medição nova no registro: `[prog]`, uma vez por minuto, quantos programas de shader o jogo descartou e quanto esperou.
- Depois do primeiro teste (no BOTW), os analógicos ficaram leves no meio do curso: na metade, cerca de um quarto do
  movimento; inteiro só na borda.
- **Jogada no S23 Ultra (05/11/2026):** tudo rodando bem, texturas boas. A grade dos quadros ainda não foi medida.

### b7: drivers numa aba, recomendado pelo aparelho (05/11/2026)

- A mesma mudança da b14 do BOTW: a aba do Snapdragon 8 Gen 2 com 9 drivers, o mais novo em cima
  ([DRIVERS.md](DRIVERS.md)). Na primeira vez, o app lê o celular e escolhe o testado: One UI em Samsung e com driver
  do sistema 676.70 ou mais novo, o padrão nos outros. Só o escolhido é copiado para o celular.
- Os de 01/11 saíram; os que já tinham sido copiados continuam na lista.
- **Testada no S23 Ultra (05/11/2026):** 15 minutos jogando com a cena a 50%, FSR 1, SMAA e o controle de temperatura
  ligado: 30 FPS em quase todas as janelas (uma de 27), nenhum erro de driver. O sistema freou a GPU de leve (até 475
  MHz) com a bateria a 31 °C e a pele a 42-45 °C: pele e chip também contam, não só a bateria.
- **Botão rápido do movimento pelos analógicos redondo e pequeno** (pedido do Moab), com um analógico desenhado, no lugar
  da pílula com texto: cobria demais a tela junto com os controles de toque.
- **Versão Universal** (pedido do Moab), como a do BOTW: `-Puniversal=1` compila sem as instruções do 8 Gen 2
  (`S23_CPU`) e leva os 26 drivers em 8 abas, com o recomendado para cada chip. Mesmo pacote: uma instala por cima da
  outra. Conferida com o `conferir_universal.py` do BOTW. Experimental: só testada no S23.

## O que não foi modificado

- O código do jogo roda nativo, como no original: o app não compila o jogo.
- Saves, arquivos do jogo, chaves de assinatura e as bibliotecas FFmpeg das vozes: nada disso faz parte do
  repositório.
