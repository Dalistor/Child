# Child

Uma tentativa de criar uma **versão bem simplificada do cérebro humano**, em C++20 e sem dependências externas.

## A ideia

A maioria das IAs de hoje é treinada uma vez, congelada e depois só usada. O cérebro é diferente: aprende de forma contínua, a partir de regras **locais** entre neurônios, e cada pessoa segue um caminho próprio a partir do que viveu.

Child parte dessa ideia. Em vez de otimizar uma rede de camadas por retropropagação, o plano é recriar, em degraus, as peças do cérebro: o **neurônio** que dispara, a **sinapse** que aprende pela ordem dos disparos (STDP), a **dopamina** que decide o que vale a pena guardar, o **hipocampo** que repete as experiências para consolidá-las. Cada peça fica numa pasta com o nome da estrutura que representa.

## Onde estamos

**As primeiras peças do cérebro já estão no código:** um neurônio AdEx que dispara e se adapta (`src/neuron/`), uma sinapse que aprende pela ordem dos disparos e se reescala para manter a taxa de disparo no alvo (`src/synapse/`), a dopamina que decide se esse aprendizado vira mudança de peso (`src/dopamine/`) e o interneurônio inibitório que impede a população de disparar sem controle (`src/interneuron/`). Por enquanto eles são ligados entre si apenas nos testes. Ao lado deles fica a base de partida, uma rede de camadas que prevê o próximo byte, com captação, aprendizado e engrama com **ramificação** (`fork`): copiar o estado e deixar cada cópia divergir sem afetar a origem. Ela segue no repositório como referência para comparar e testar o que vier.

| Degrau | O que entrega | Situação |
|---|---|---|
| Base MLP | Captação, aprendizado, engrama em seções e ramificação; build reprodutível | feito (referência) |
| 0 · [`cerebro`](.centaur/contracts/cerebro/v002.json) | Neurônio AdEx, sinapse com STDP, dopamina, homeostase e inibição; um neurônio que aprende um padrão | em andamento: neurônio, sinapse, dopamina, homeostase e inibição feitos; falta o aprendizado de um padrão |
| 1 · Dopamina como surpresa | O erro ao prever o próximo byte libera a plasticidade | planejado |
| 2 · Cerebelo | Leitura do próximo byte sem retropropagação | planejado |
| 3 · Segmentos dendríticos | Um neurônio resolve XOR; contextos de sequência separados | planejado |
| 4 · Hipocampo e `sleep` | Aprender algo novo sem esquecer o antigo | planejado |
| 5 · Escala e GPU | Muitos neurônios, CUDA opcional, engrama com estado dinâmico | planejado |

O roteiro e o critério de cada degrau estão na [síntese da pesquisa](docs/sintese.md).

Estrutura-alvo, por regiões do cérebro: `thalamus/` (entrada sensorial), `neuron/`, `synapse/`, `interneuron/`, `dopamine/`, `cerebellum/`, `hippocampus/` e `engram/` (armazenamento, já entregue).

## Seja realista

O cérebro humano tem 86 bilhões de neurônios e de 10¹⁴ a 10¹⁵ sinapses. Um neurônio humano real é bem mais complexo que o artificial clássico. O objetivo aqui é uma versão **bem simplificada**, e uma GPU como a desta máquina deve simular, segundo benchmarks de outra GPU, na casa de centenas de milhares de neurônios simples, não de bilhões (ainda não medimos aqui). A [pesquisa](docs/pesquisa-cerebro.md) por trás de cada decisão, com fontes, está registrada.

## Do clone ao primeiro fork treinado

Precisa de um compilador C++20 e CMake (Ninja opcional).

```bash
# 1. Clonar, compilar e testar
git clone https://github.com/Dalistor/Child.git && cd Child
cmake -S . -B build -G Ninja && cmake --build build   # portátil: mesma semente, mesmos pesos em qualquer máquina
ctest --test-dir build

# 2. Criar a base virgem e uma ramificação sua
./build/child init root.child --name root --seed 1
./build/child fork root.child meu.child --name meu

# 3. Ensinar a ramificação (qualquer arquivo ou pasta; aqui, o próprio código)
./build/child feed meu.child src --epochs 3

# 4. Ver a linhagem: a ramificação aprendeu, a raiz continua virgem
./build/child info meu.child
./build/child info root.child

# 5. Gerar texto a partir do que ela aprendeu
./build/child sample meu.child --prompt "int "
```

Os arquivos `.child` são engramas: o estado completo de uma base. Ramifique quantas vezes quiser; cada cópia diverge sem afetar a origem. Para contribuir, veja o [guia de contribuição](CONTRIBUTING.md).

## Documentação

- [Arquitetura](.centaur/system/architecture.md): o estado atual e a direção.
- [Pesquisa](docs/README.md): comece pela [síntese](docs/sintese.md); neurônio humano, aprendizado local, esquecimento, escala e ideias, com fontes.
- [Contratos](.centaur/contracts): o comportamento esperado, regra por regra, com as [evidências](.centaur/evidence) dos testes da base.
- [AGENTS.md](AGENTS.md): instruções para agentes de IA que trabalham no projeto, que usa o fluxo [Centaur](.centaur/workspace.json).
- [CONTRIBUTING.md](CONTRIBUTING.md): como compilar, testar e propor mudanças.

## Licença

[MIT](LICENSE).
