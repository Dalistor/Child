# Child

Uma tentativa de criar uma **versão bem simplificada do cérebro humano**, em C++20 e sem dependências externas.

## A ideia

A maioria das IAs de hoje é treinada uma vez, congelada e depois só usada. O cérebro é diferente: aprende de forma contínua, a partir de regras **locais** entre neurônios, e cada pessoa segue um caminho próprio a partir do que viveu.

Child parte dessa ideia. Em vez de otimizar uma rede de camadas por retropropagação, o plano é recriar, em degraus, as peças do cérebro: o **neurônio** que dispara, a **sinapse** que aprende pela ordem dos disparos (STDP), a **dopamina** que decide o que vale a pena guardar, o **hipocampo** que repete as experiências para consolidá-las. Cada peça fica numa pasta com o nome da estrutura que representa.

## Onde estamos

**O cérebro simplificado já funciona de ponta a ponta e é o motor padrão.** Um byte entra pelo **tálamo** (`src/thalamus/`), que o codifica em disparos. Ele chega a um **córtex** (`src/cortex/`) de neurônios AdEx, com:
- sinapses que aprendem por STDP, moduladas pela **dopamina**;
- escala homeostática;
- interneurônios inibitórios;
- segmentos dendríticos.

O **cerebelo** (`src/cerebellum/`) lê essa atividade e prevê o próximo byte por regra delta local. A **surpresa** dessa previsão é a dopamina que libera a plasticidade. O **hipocampo** (`src/hippocampus/`) guarda episódios e os repete no `sleep`, para aprender o novo sem esquecer o antigo. Quando a surpresa fica alta e para de cair, a base cria **neurônios novos** (neurogênese), até um teto. Tudo fica num engrama com o estado dinâmico completo, que pode ser ramificado com `fork`. A rede de camadas original (MLP) segue disponível com `--engine mlp`, como oráculo dos testes.

| Degrau | O que entrega | Situação |
|---|---|---|
| Base MLP | Captação, aprendizado, engrama em seções e ramificação; build reprodutível | feito (oráculo) |
| 0 · [`cerebro`](.centaur/contracts/cerebro/v002.json) | Neurônio AdEx, STDP, dopamina, homeostase, inibição; um neurônio aprende um padrão | feito |
| 1 · [`encefalo`](.centaur/contracts/encefalo/v001.json) | Dopamina como surpresa, acompanhando a perda do MLP | feito |
| 2 · [`encefalo`](.centaur/contracts/encefalo/v001.json) | Cerebelo: leitura do próximo byte sem retropropagação | feito |
| 3 · [`dendritos`](.centaur/contracts/dendritos/v001.json) | Um neurônio resolve XOR; contextos de sequência separados | feito |
| 4 · [`hipocampo`](.centaur/contracts/hipocampo/v001.json) | `sleep`: aprender B sem estragar A | feito |
| 5 · [`engrama`](.centaur/contracts/engrama/v001.json), [`neurogenese`](.centaur/contracts/neurogenese/v001.json) | Engrama com estado dinâmico, crescimento de neurônios, vazão medida | feito em CPU; GPU (CUDA) é extensão |

Os critérios de cada degrau estão nos contratos e na [síntese da pesquisa](docs/sintese.md). As limitações conhecidas estão na [arquitetura](.centaur/system/architecture.md#limites-conhecidos).

## Seja realista

O cérebro humano tem 86 bilhões de neurônios e de 10¹⁴ a 10¹⁵ sinapses. O Child simula por padrão 256 células piramidais e 64 interneurônios, a cerca de 500 bytes por segundo numa thread de CPU (1,7·10⁷ passos de neurônio por segundo, medidos com `child bench`). A qualidade do texto gerado não é o objetivo. Os testes medem mecanismos: aprender um padrão, prever acima do acaso, resolver XOR, separar contextos e não esquecer. A [pesquisa](docs/README.md) por trás de cada decisão, com fontes, está registrada.

## Do clone ao primeiro fork treinado

Precisa de um compilador C++20 e CMake (Ninja opcional).

```bash
# 1. Clonar, compilar e testar
git clone https://github.com/Dalistor/Child.git && cd Child
cmake -S . -B build -G Ninja && cmake --build build   # portátil: mesma semente, mesmo cérebro em qualquer máquina
ctest --test-dir build

# 2. Criar a base virgem (um cérebro) e uma ramificação sua
./build/child init root.child --name root --seed 1
./build/child fork root.child meu.child --name meu

# 3. Ensinar a ramificação (qualquer arquivo ou pasta; aqui, parte do próprio código) e dormir
./build/child feed meu.child src/cortex --epochs 2 --sleep 1

# 4. Ver a linhagem: a ramificação aprendeu, a raiz continua virgem
./build/child info meu.child
./build/child info root.child

# 5. Gerar texto a partir do que ela aprendeu
./build/child sample meu.child --prompt "int "
```

Os arquivos `.child` são engramas: o estado completo de uma base, incluindo potenciais, traços e marcas de elegibilidade. Ramifique quantas vezes quiser; cada cópia diverge sem afetar a origem.

## Comandos e variáveis de ambiente

| Comando | O que faz |
|---|---|
| `child init <ckpt> [--params FILE] [--neurons N] [--engine brain\|mlp]` | Cria uma base virgem |
| `child fork <src> <dst> --name N` | Ramifica |
| `child feed <ckpt> <path> [--epochs N] [--sleep C]` | Ensina; pode criar neurônios quando precisa |
| `child sleep <ckpt> [--cycles N]` | Repete os episódios do hipocampo |
| `child grow <ckpt> --neurons N` | Cria neurônios à mão, até o teto |
| `child sample <ckpt> [--prompt T]` | Gera bytes |
| `child info <ckpt>` / `child params [<ckpt>]` | Linhagem e tamanho / parâmetros com faixa e fonte |
| `child bench [--neurons N]` | Mede a vazão |

| Variável | Efeito |
|---|---|
| `CHILD_NEUROGENESIS` | `on` (padrão) ou `off`: se a base pode criar neurônios ao aprender, nesta execução |
| `CHILD_MAX_NEURONS` | Teto de células piramidais nesta execução (padrão: `neurogenesis.max_neurons` da base, 1024) |
| `CHILD_PARAMS` | Arquivo de parâmetros que o `init` lê quando falta `--params` |
| `CHILD_ENGINE` | Motor que o `init` cria: `brain` (padrão) ou `mlp` |

O arquivo de parâmetros tem uma linha `nome = valor` por parâmetro, e `#` inicia um comentário. `child params` lista os nomes. Um valor fora da faixa registrada é recusado, com o nome do parâmetro na mensagem. Para contribuir, veja o [guia de contribuição](CONTRIBUTING.md).

## Documentação

- [Arquitetura](.centaur/system/architecture.md): o estado atual e a direção.
- [Pesquisa](docs/README.md): comece pela [síntese](docs/sintese.md); neurônio humano, aprendizado local, esquecimento, escala e ideias, com fontes.
- [Contratos](.centaur/contracts): o comportamento esperado, regra por regra, com as [evidências](.centaur/evidence) dos testes da base.
- [AGENTS.md](AGENTS.md): instruções para agentes de IA que trabalham no projeto, que usa o fluxo [Centaur](.centaur/workspace.json).
- [CONTRIBUTING.md](CONTRIBUTING.md): como compilar, testar e propor mudanças.

## Licença

[MIT](LICENSE).
