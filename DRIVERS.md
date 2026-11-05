# Drivers Vulkan do TOTK S23

A partir da b7, os drivers ficam numa aba por chip, com o mais novo em cima. A versão S23 é feita para o Snapdragon 8 Gen 2, por isso só traz a aba dele, com 9 drivers. A versão **Universal** traz os 26 drivers em 8 abas, com o recomendado para cada chip; a tabela dos recomendados está no DRIVERS.md do BOTW, logo abaixo.

Um driver só é copiado para o celular quando você escolhe. Os outros não ocupam espaço.

A pesquisa completa está no [DRIVERS.md da versão S23 do BOTW](https://github.com/Moab-tk/BOTWRecompAndroid-S23/blob/s23/DRIVERS.md), que usa o mesmo motor. Lá estão:

- os relatos de jogadores nas issues do original;
- os repositórios de onde cada driver vem;
- as recomendações para os outros chips.

## Como o app escolhe

Na primeira vez que o app abre, ele identifica o celular:

- a marca e o sistema;
- o chip;
- a GPU;
- a versão do driver do próprio sistema.

Num 8 Gen 2, ele escolhe entre duas versões do mesmo driver, as duas testadas no S23 Ultra:

- **Banners Turnip 27.0.0 (One UI):** em celulares Samsung e em qualquer um com o driver do sistema na versão 676.70 ou mais nova. O S23 Ultra está na 676.79. Nesses celulares, sem esse ajuste, a tela pisca.
- **Banners Turnip 27.0.0 padrão:** nos outros.

Depois disso, vale a sua escolha.

## Os 9 drivers

| Driver | Autor | Data |
|---|---|---|
| Banners Turnip 27.0.0 r5 (One UI) | [The412Banner](https://github.com/The412Banner/Banners-Turnip) | 05/11/2026 |
| Banners Turnip 27.0.0 r5 | The412Banner | 05/11/2026 |
| Banners Turnip 27.0.0 (One UI, testado) | The412Banner | 25/10/2026 |
| Banners Turnip 27.0.0 (testado) | The412Banner | 25/10/2026 |
| StevenMXZ Turnip 27.0.0 R1 (One UI) | [StevenMXZ](https://github.com/StevenMXZ/Adreno-Tools-Drivers) | 19/10/2026 |
| StevenMXZ Turnip 27.0.0 R1 | StevenMXZ | 19/10/2026 |
| StevenMXZ Turnip Gen8 V38 Patched | StevenMXZ | 19/10/2026 |
| Balemuni Apex 2 Ultimate (8 Gen 2) | [Balemuni](https://github.com/Balemuni/Balemunis-Aurora) | 26/08/2026 |
| Mr Purple Turnip T30 | [MrPurple666](https://github.com/MrPurple666/purple-turnip) | 17/08/2026 |

Só os dois marcados como "testado" foram testados neste jogo. O Gen8 V38 Patched e o T30 foram os que os jogadores do BOTW mais elogiaram no S23. Para saber se algum deles roda melhor no TOTK, é preciso testar.

Todos são o driver Turnip da [Mesa](https://docs.mesa3d.org/license.html), sob a licença MIT e outras. As somas SHA-256 estão em `android/totkrecompandroid/app/src/drivers/LEIA-ME.txt`.

Os drivers da Qualcomm não vêm no app, porque não podem ser redistribuídos. Quem quiser usa o botão **Adicionar driver**.

---

From b7 on, the drivers are in a tab per chip. The S23 version only has the Snapdragon 8 Gen 2 tab, with 9 drivers, newest first; the Universal one has all 26 in 8 tabs, with the recommended one per chip. A driver is copied out only when it's chosen.

On the first start, the app picks the tested Banners 27.0.0 build. It uses the One UI build on Samsung phones and on any 8 Gen 2 whose system driver is 676.70 or newer; on the others it uses the standard one. Afterwards your choice stays.

The full research is in the [BOTW S23 DRIVERS.md](https://github.com/Moab-tk/BOTWRecompAndroid-S23/blob/s23/DRIVERS.md).
