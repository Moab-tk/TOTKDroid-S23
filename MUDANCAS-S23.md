# O que mudou em relação ao original

Projeto original: [TOTKDroid](https://github.com/mamcjr/TOTKDroid), de mamcjr. Todas as versões, da b1 à b5, partem da
`0.2.1-beta`. A versão de cada etapa está no histórico do repositório (ramo `s23`).

O motor é o mesmo do BOTWRecompAndroid. Muito do que está aqui veio da versão S23 do BOTW
([BOTWRecompAndroid-S23](https://github.com/Moab-tk/BOTWRecompAndroid-S23)), conferido trecho por trecho.

## Resumo por área

| Área | O que mudou |
|---|---|
| Pacote | `com.totkdroid.s23`, instala ao lado do original; nome do app "TOTK S23" |
| Compilação | Feita para o processador do Snapdragon 8 Gen 2 |
| Ritmo dos quadros | Tela em 60 Hz com quadros presos ao ritmo real dela (quadros regulares a 30 FPS) |
| Processador | Orçamento de 30 FPS avisado ao Android (ADPF) |
| Shaders | Quem espera faz a tarefa, tabelas maiores, registro das esperas longas, montagem antecipada |
| Drivers | Cinco drivers Turnip dentro do app, com licenças; o recomendado escolhido pelo celular |
| Configurações | Recomendadas com um toque, 8 blocos salvos, gravação segura |
| Interface | Maior, em cartões, tamanho do texto ajustável, textos em português |
| Calor | Controle de temperatura opcional, com a escala da cena em tamanhos inteiros (b5) |
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
- **Ainda não foi testada jogando.** Por isso a versão na aba Releases é a b4; a b5 vai para lá depois do teste.

## O que não foi modificado

- O código do jogo roda nativo, como no original: o app não compila o jogo.
- Saves, arquivos do jogo, chaves de assinatura e as bibliotecas FFmpeg das vozes: nada disso faz parte do
  repositório.
