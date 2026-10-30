# Child

Uma tentativa de criar uma **versão bem simplificada do cérebro humano**, em C++20 e sem dependências externas.

## A ideia

A maioria das IAs de hoje é treinada uma vez, congelada e depois só usada. O cérebro é diferente: aprende de forma contínua, a partir de regras **locais** entre neurônios, e cada pessoa segue um caminho próprio a partir do que viveu.

Child parte dessa ideia. Em vez de otimizar uma rede de camadas por retropropagação, o plano é recriar, em degraus, as peças do cérebro: o **neurônio** que dispara, a **sinapse** que aprende pela ordem dos disparos (STDP), a **dopamina** que decide o que vale a pena guardar, o **hipocampo** que repete as experiências para consolidá-las. Cada peça fica numa pasta com o nome da estrutura que representa.

## Onde estamos

**Hoje ainda não há neurônios no código.** O que existe é uma base de partida, uma rede de camadas que prevê o próximo byte, com captação, aprendizado e armazenamento com **ramificação** (`fork`): copiar o estado e deixar cada cópia divergir sem afetar a origem. Ela segue no repositório como referência para comparar e testar o que vier.

| Degrau | O que entrega | Situação |
|---|---|---|
| Base MLP | Captação, aprendizado, checkpoint e ramificação | feito (referência) |
| [`cerebro`](.centaur/contracts/cerebro/v001.json) | Neurônio AdEx, sinapse com STDP, dopamina, um neurônio que aprende um padrão | contrato aprovado, sem código |
| Neurônio humano | Dendritos com NMDA e picos de cálcio; um único neurônio resolvendo XOR | planejado |
| Hipocampo | Aprendizado rápido e replay, contra o esquecimento catastrófico | planejado |
| Escala e GPU | Muitos neurônios, CUDA opcional | planejado |

Estrutura-alvo, por regiões do cérebro: `thalamus/` (entrada sensorial), `neuron/`, `synapse/`, `dopamine/`, `hippocampus/` e `engram/` (armazenamento).

## Seja realista

O cérebro humano tem 86 bilhões de neurônios e de 10¹⁴ a 10¹⁵ sinapses. Um neurônio humano real é bem mais complexo que o artificial clássico. O objetivo aqui é uma versão **bem simplificada**, e uma GPU como a desta máquina deve simular, segundo benchmarks de outra GPU, na casa de centenas de milhares de neurônios simples, não de bilhões (ainda não medimos aqui). A [pesquisa](.centaur/system/pesquisa-cerebro.md) por trás de cada decisão, com fontes, está registrada.

## Como usar a base atual

Precisa de um compilador C++20 e CMake.

```bash
cmake -S . -B build -G Ninja && cmake --build build   # compilar
ctest --test-dir build                                # testes

./build/child init root.child --name root             # base virgem
./build/child fork root.child a.child --name a        # ramificação
./build/child feed a.child src --epochs 3             # captar e aprender
./build/child info a.child                            # linhagem e estado
./build/child sample a.child --prompt "int "          # gerar texto
```

Depois do `feed`, só `a.child` terá bytes aprendidos; `root.child` continua virgem.

## Documentação

- [Arquitetura](.centaur/system/architecture.md): o estado atual e a direção.
- [Pesquisa](.centaur/system/pesquisa-cerebro.md): neurônio humano, modelos, aprendizado, esquecimento e escala.
- [Contratos](.centaur/contracts): o comportamento esperado, regra por regra, com as [evidências](.centaur/evidence) dos testes da base.
- [AGENTS.md](AGENTS.md): instruções para agentes de IA que trabalham no projeto, que usa o fluxo [Centaur](.centaur/workspace.json).
