# TOTK S23

**Português** | [English](README.en.md)

Modificação **não oficial** do [TOTKDroid](https://github.com/mamcjr/TOTKDroid), ajustada para o **Samsung Galaxy S23
Ultra** (Snapdragon 8 Gen 2, Adreno 740).

> [!IMPORTANT]
> **O projeto original é de [mamcjr](https://github.com/mamcjr), criador do
> [TOTKDroid](https://github.com/mamcjr/TOTKDroid), a partir do LAS Recomp de [peiikai](https://github.com/peiikai).
> Todo o crédito do projeto é deles.**
>
> Esta versão é uma modificação pessoal feita por outra pessoa, com ajuda de IA. mamcjr não participou destas
> mudanças, não as testou e não responde por elas.

> [!CAUTION]
> **Problema nesta versão? Abra uma [issue aqui](https://github.com/Moab-tk/TOTKDroid-S23/issues). Nunca leve estas
> reclamações ao projeto original.** Um erro nesta versão pode ter sido criado por estas modificações e pode não existir
> no original. Ela pode funcionar bem no celular de quem testou e funcionar mal no seu.

> [!NOTE]
> Não leve nada a sério. Se não der certo no seu celular, é só deixar para lá e usar a versão que achar melhor.

> [!WARNING]
> **O celular dos testes é modificado.** É um S23 Ultra de 12 GB, com overclock, com o limitador térmico do sistema
> desativado por ferramentas de terceiros, com outras modificações e com cooler. Nenhum número desta página vale como
> promessa para um celular comum.

O texto original do projeto, com todos os detalhes técnicos, está em [README-ORIGINAL.pt-BR.md](README-ORIGINAL.pt-BR.md)
(e em inglês, [README-ORIGINAL.md](README-ORIGINAL.md)). A lista do que foi modificado está em
[MUDANCAS-S23.md](MUDANCAS-S23.md).

## Sem arquivos do jogo

**Nenhum arquivo do jogo vem incluído.** Você precisa do seu próprio dump, obtido legalmente, de *Tears of the Kingdom*
com a atualização **1.4.3** aplicada (`exefs` e `romfs`). Não serão dadas instruções de como obtê-lo.

## O que esta versão tem de diferente

- **Compilada para o processador do S23**, sem as instruções que ele não tem.
- **Instala ao lado do original:** o pacote é outro (`com.totkdroid.s23`), então uma versão não substitui a outra.
- **Baseada na versão 0.2.1 do original**, a mais nova até agora.
- **Trazido da versão S23 do BOTW** ([BOTWRecompAndroid-S23](https://github.com/Moab-tk/BOTWRecompAndroid-S23)), que
  usa o mesmo motor:
  - quadros regulares a 30 FPS, com a tela em 60 Hz e cada quadro seguindo o ritmo real dela;
  - orçamento de 30 FPS avisado ao Android, para ele não baixar o processador além da conta;
  - shaders: quem espera faz a tarefa, tabelas maiores e montagem antecipada dos pipelines já vistos;
  - cinco drivers Turnip dentro do app, configurações recomendadas com um toque e 8 blocos de configurações salvas;
  - interface maior, em cartões, com textos em português;
  - revisão do código: esperas no relógio certo, configurações gravadas com segurança, conferências no kernel e na
    linha de desenho.
- **Controle de temperatura (opcional, desligado por padrão):** reduz a cena por etapas antes de o sistema cortar a
  GPU. Na b4 ele fazia cacos nas bordas e o sol estourado; a b5 corrige o motivo (veja em
  [MUDANCAS-S23.md](MUDANCAS-S23.md)).
- **Sensibilidade do arrasto da câmera até 800%.**

## Instalando

Você precisa de Android 11 ou mais novo, de um celular arm64 com Vulkan e de espaço para os arquivos do jogo.

1. Baixe o APK na aba **Releases** desta página e instale.
2. Abra e escolha a pasta com o seu `exefs` e `romfs` da 1.4.3. O jogo roda em código nativo: o app não compila o jogo.
3. Na tela inicial, abra **Configurações salvas** e toque em **Usar configurações recomendadas**.
4. Confirme. O app reinicia e o jogo está pronto para jogar.

Atualizando uma versão anterior desta modificação, basta instalar por cima: os saves e as configurações continuam.

## Configurações recomendadas

| Opção | S23 Ultra | S23, S23+ e mais fracos |
|---|---|---|
| Limite de quadros | 30 FPS | 30 FPS |
| Resolução interna | 1x | 75% |
| Resolução de saída | 1080 linhas | 900 linhas |
| Ampliação da imagem | FSR 1 | FSR 1 |
| Suavização | Desligada | Desligada |
| Código do jogo | Nativo | Nativo |
| Quadros regulares a 30 FPS | Ligado | Ligado |
| Controle de temperatura | Desligado | Desligado |

A opção mais leve não foi testada no S23, no S23+ nem em celulares mais fracos.

## Desempenho medido

Números de um único aparelho **modificado**, jogo 1.4.3, código nativo, 30 FPS.

| Medição | Configuração | FPS médio | Observações |
|---|---|---:|---|
| b1, parte guiada | GPU limitada pelo sistema | 25,5 | 67% dos quadros no tempo certo |
| b1, parte guiada | GPU liberada | 26,8 | 75% dos quadros no tempo certo |
| b4, 30 minutos jogando | 1x, saída 1080, calor desligado | 27,7 | 71% das medições em 27 ou mais; 5% abaixo de 20 |
| b4, dentro de um santuário | a mesma | 29,8 | quase sem quedas |

Nas áreas abertas e cheias o jogo manda de 4 a 5 mil desenhos por quadro. Ali o limite é o processador, não a GPU:
baixar a resolução ajuda pouco. Na mesma partida de 30 minutos, o sistema baixou o limite do núcleo mais rápido de
3.360 para 1.555 MHz pelo calor, e o FPS caiu junto.

## Problemas conhecidos

- **Versão para baixar: b4**, a testada jogando. Nela, deixe o controle de temperatura desligado (o padrão): ligado, ele
  causa cacos nas bordas e o sol estourado. A correção disso e a sensibilidade até 800% (b5) já estão no código deste
  ramo, mas a b5 só vai para a aba Releases depois de testada.
- **Quedas para 17 a 24 FPS em áreas abertas:** o processador não dá conta de tantos desenhos, e o calor piora.
  O próximo passo é medir com o perfilador onde a linha de desenho gasta o tempo.
- **Memória:** num celular de 12 GB o jogo chegou a 9,2 GB de memória ocupada e encheu a memória de troca na primeira
  sessão. Celulares com menos memória podem fechar o jogo.
- **Montagem antecipada:** a primeira sessão depois de instalar uma versão nova só anota os pipelines; o ganho aparece
  da segunda em diante.
- **Só testada no S23 Ultra.** Não há versão Universal do TOTK por enquanto.

## Opções de teste

Chaves `LARECOMP_*` só funcionam com o arquivo `testes.txt` na pasta do app, que só o `adb` consegue criar. Algumas
úteis: `LARECOMP_SCALE_SNAP=0` (escala da cena como antes da b5), `LARECOMP_NO_PREWARM=1` (sem montagem antecipada),
`LARECOMP_LOG_HEAT=1` (registro do calor a cada 30 s).

## Apoie, se quiser

Doação voluntária, sem nenhum benefício, para ajudar com custos como os tokens do Claude. Ela vai para Moab, autor desta
modificação, e não para mamcjr, peiikai ou a Nintendo. Pix (chave aleatória): `5eb148c4-81dc-47cd-b2cc-b3171d3c06ab`

Para apoiar o projeto original, use o [patrocínio de mamcjr no GitHub](https://github.com/sponsors/mamcjr).

## Compilando

Igual ao original: veja [README-ORIGINAL.pt-BR.md](README-ORIGINAL.pt-BR.md). A chave de assinatura fica fora do
repositório (`~/.totkrecompandroid/keystore.properties`).

## Créditos

- [mamcjr](https://github.com/mamcjr), pelo [TOTKDroid](https://github.com/mamcjr/TOTKDroid).
- [peiikai](https://github.com/peiikai), pelo LAS Recomp (licença MIT, em [LICENSE](LICENSE)).
- Todos os créditos do original continuam valendo: veja [README-ORIGINAL.pt-BR.md](README-ORIGINAL.pt-BR.md#créditos).
- Drivers Turnip: builds do Mesa feitas por [The412Banner](https://github.com/The412Banner/Banners-Turnip).
