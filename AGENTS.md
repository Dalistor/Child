# Child

## Visão Geral
O objetivo é criar uma **versão bem simplificada do cérebro humano**, em C++20 e sem dependências externas. O Child é uma **base** que outras pessoas clonam e treinam. A direção, em degraus, está na [síntese](docs/sintese.md):
0. neurônio AdEx, sinapse com STDP, homeostase e inibição;
1. dopamina como surpresa;
2. leitura cerebelar do próximo byte;
3. segmentos dendríticos;
4. hipocampo com `sleep`;
5. escala e engrama de estado.

Todos estão entregues em CPU. A neurogênese cria neurônios quando a base precisa, sob controle de variáveis de ambiente.

O **cérebro** (`src/brain/`) é o motor padrão do CLI. Ele liga tálamo, córtex com dendritos e inibição, cerebelo, dopamina, hipocampo e neurogênese. O **MLP** (`src/model/`) segue como oráculo dos testes e motor alternativo (`--engine mlp`); **não é um neurônio**. Detalhes, decisões e limites conhecidos estão na [arquitetura](.centaur/system/architecture.md).

## Pesquisa
Toda pesquisa (cérebro, redes neurais, ideias) fica em [docs/](docs/README.md), com fontes e a data. Pesquisa nova vira arquivo novo ali e entra no índice; não vai para `.centaur/`.

## Recuperação de contexto
Priorize busca e símbolos do editor. Graphify é opcional para relações amplas; consulte `.centaur/graphify/graph.json` pelo adaptador da skill apenas quando útil. Confirme fontes atuais. Não indexe automaticamente após mudanças. Decisões e documentação gerada ficam em `.centaur/`; código e testes comprovam comportamento.

## Memória de decisões e mudanças
Backend `files` (confirmado em `.centaur/workspace.json`): registros em `.centaur/implements/` com índice em [status.md](.centaur/implements/status.md). Chame a skill `centaur-driven-memory` para consultar ou registrar. Memória histórica não substitui código, testes, regras ou o estado das specs.

## Contratos e realização
Contratos em `.centaur/contracts/<id>/vNNN.json` definem o comportamento desejado e a autonomia. Estado em `.centaur/state/` e evidências em `.centaur/evidence/` registram a realização; o ciclo normativo de `centaur-driven-graphify` define formatos e gates. Contratos aprovados são versionados, não reescritos. Antes de editar, consultar apenas as regras e limites pertinentes. Checklist de spec não comprova comportamento. Consulte specs, fontes e evidências diretamente no terminal. Apresente resultado, validação e próxima ação com links; detalhes ficam nos registros.

Contratos aprovados:
- [`cerebro` v002](.centaur/contracts/cerebro/v002.json) (2026-11-02; v001 de 2026-10-30): neurônio AdEx (`src/neuron/`), sinapse com STDP e escala homeostática (`src/synapse/`), dopamina como terceiro fator (`src/dopamine/`), interneurônio (`src/interneuron/`), aprendizado de um padrão (APR-01), parâmetros com faixa (PAR-01, `tests/parameters_doc.py`) e arquivo de parâmetros (CFG-01, `src/brain/parameters`). Estado em [cerebro.json](.centaur/state/cerebro.json).
- [`encefalo`](.centaur/contracts/encefalo/v001.json) (2026-11-07): o cérebro integrado. Tálamo (`src/thalamus/`), córtex (`src/cortex/`), cerebelo (`src/cerebellum/`), dopamina como surpresa, CLI e vazão medida. Estado em [encefalo.json](.centaur/state/encefalo.json).
- [`dendritos`](.centaur/contracts/dendritos/v001.json), [`hipocampo`](.centaur/contracts/hipocampo/v001.json) e [`neurogenese`](.centaur/contracts/neurogenese/v001.json) (2026-11-07): XOR num neurônio e contextos de sequência; `sleep` contra o esquecimento e completação; criação de neurônios com teto e variáveis de ambiente.
- [`engrama`](.centaur/contracts/engrama/v001.json) (2026-11-02): seções versionadas (ENG-01) e estado dinâmico (ENG-02, seção `brain`).
- [`base`](.centaur/contracts/base/v002.json) (2026-10-29): o MLP, referência e oráculo.
- [`reprodutibilidade`](.centaur/contracts/reprodutibilidade/v001.json) e [`base-aberta`](.centaur/contracts/base-aberta/v001.json) (2026-11-02): build portátil com hash de referência; licença, guia e exemplo de ponta a ponta.

Os rascunhos `memoria-eficiente` e `escala-gpu` foram **retirados** (falavam de milhões de parâmetros de um MLP). O que se aproveita deles está na [pesquisa](docs/pesquisa-cerebro.md).

## Comandos essenciais
```bash
cmake -S . -B build -G Ninja && cmake --build build   # compilar (Release portátil; -DCHILD_NATIVE=ON otimiza para a CPU, sem reprodutibilidade)
ctest --test-dir build                                # child_tests, cli_branching, cli_brain, brain_layout, parameters_doc
./build/child init root.child --name root             # cérebro virgem (--engine mlp para o oráculo; --params FILE)
./build/child fork root.child a.child --name a        # ramificar
./build/child feed a.child src/cortex --epochs 2 --sleep 1   # captar, aprender e dormir
./build/child info a.child                            # linhagem e tamanho
./build/child sample a.child --prompt "int "          # gerar bytes
./build/child params                                  # parâmetros, faixas e fontes
./build/child bench --neurons 1024                    # vazão
python3 <skills>/centaur-driven-graphify/scripts/validate-lifecycle.py .   # validar registros Centaur
```
Variáveis de ambiente:
- `CHILD_NEUROGENESIS=on|off` e `CHILD_MAX_NEURONS=N` valem em cada execução;
- `CHILD_PARAMS=FILE` e `CHILD_ENGINE=brain|mlp` valem no `init`.

Rode os comandos a partir da raiz do projeto. Arquivos `.child` são checkpoints; não os versione.

## Arquitetura de Camadas
As pastas de `src/` são nomeadas pelas estruturas do cérebro, cada uma com um arquivo principal de mesmo nome, verificado por `tests/brain_layout.py`. A tabela completa está na [arquitetura](.centaur/system/architecture.md#módulos-e-dependências).

| Camada | Pastas | Responsabilidade | Proibido |
|---|---|---|---|
| Núcleo | `neuron/`, `synapse/`, `dopamine/`, `interneuron/` | Dinâmica de uma célula, sinapse ou sinal | E/S; conhecer regiões, cérebro ou engrama |
| Regiões | `thalamus/`, `cortex/`, `cerebellum/`, `hippocampus/` | Uma estrutura do cérebro, montada com o núcleo | E/S; conhecer `brain/` ou engrama |
| Encéfalo | `brain/` | Liga as regiões, parâmetros com faixa, neurogênese, serialização | E/S de arquivos; conhecer engrama ou CLI |
| Engrama | `engram/` | Seções, linhagem, `fork`, escrita atômica | Interpretar a seção `brain`; alterar pesos |
| Legadas | `capture/`, `model/` | Fonte → bytes; MLP oráculo e `Rng` | Conhecer o cérebro |
| CLI | `src/main.cpp` | Orquestra as camadas; lê as variáveis de ambiente | Lógica de aprendizado ou de formato |

Dependências apontam para dentro. Peça nova do cérebro entra no estado salvo pelo seu `persist()` e, se mudar o formato da seção `brain`, sobe a versão dessa seção em `engram.cpp`. Só mudança do cabeçalho sobe `kVersion`.

## Testes
Harness próprio em [tests/harness.hpp](tests/harness.hpp), com o executável `child_tests`:
- [tests/tests.cpp](tests/tests.cpp): MLP, núcleo, APR-01 e engrama;
- [tests/brain_tests.cpp](tests/brain_tests.cpp): o cérebro integrado.

Pelo `ctest` rodam também `cli_branching.sh` (os dois motores), `cli_brain.sh` (comandos, arquivo de parâmetros e variáveis de ambiente), `brain_layout.py` e `parameters_doc.py`. A suíte leva cerca de 40 s. Nenhuma meta de cobertura. Os testes medem mecanismos: gradientes, aprendizado de um padrão, transmissão, homeostase, inibição, leitura cerebelar, dopamina como surpresa, XOR, contextos, sono, completação, neurogênese, estado dinâmico, parâmetros, integridade do checkpoint e isolamento entre ramificações.

Teste novo só para falha concreta em ponto vital ainda sem proteção; prefira atualizar um caso existente. Mudanças de baixo impacto: compilar e rodar o `ctest`.

## Vocabulário e Idioma do Código
Código, identificadores e comentários em inglês; documentação e conversa em português. Termos: *virgin network* = base virgem (pesos aleatórios, nada aprendido); *fork* = ramificação; *lineage* = linhagem (id, pai, nome, bytes aprendidos); *capture* = captação; *window/context* = janela de bytes anteriores. Do cérebro: *neuron* = neurônio; *synapse* = sinapse; *thalamus/relay* = tálamo/célula relé; *cortex* = córtex; *cerebellum/granule/Purkinje* = cerebelo/grânulo/Purkinje; *segment* = segmento dendrítico; *neurogenesis* = neurogênese; *spike* = disparo; *STDP* = plasticidade dependente do intervalo entre disparos; *dopamine* = dopamina, o terceiro fator da plasticidade; *hippocampus* = hipocampo; *replay* = repetição de experiências; *engram* = traço físico da memória (o checkpoint). Estruturas novas são nomeadas pela região ou célula do cérebro que representam, pasta e arquivo principal com o mesmo nome.

## Regras e Restrições
- O aprendizado do código novo usa regras locais e online (STDP, três fatores, regra delta local no cerebelo, regras hebbianas nos segmentos); sem retropropagação.
- Sem bibliotecas externas: só a biblioteca padrão do C++20. CUDA, quando vier, é um backend opcional que o núcleo não conhece.
- Compilar sem avisos com `-Wall -Wextra -Wpedantic`.
- Preserve o determinismo: mesma semente, mesmos pesos, também entre máquinas (usa `Rng` próprio, não `<random>` para pesos; build com `-ffp-contract=off`; o teste `reference weights` fixa o hash dos pesos, e mudá-lo exige justificar).
- Escrita de checkpoint é atômica (`.tmp` + `rename`); nunca escreva direto no destino.
Carregue `clean-code` e as referências pertinentes ao planejar, implementar ou revisar código. Preserve o modo TDD/direto e validação proporcional definidos pelo Centaur. As instruções do usuário e deste projeto prevalecem. Consulte `.clean/` se existir, sem escrevê-lo nestes fluxos.

## Módulos e Equipe
Modo **individual**: o desenvolvedor acumula responsabilidade e integração. Escopo único `master`.
Consulte `.centaur/workspace.json` para escopos, caminhos e responsáveis. Specs ficam em `.centaur/specs`; implementações seguem o backend de memória, preservando os diretórios legados; leia seus índices sob demanda e confirme status nos READMEs canônicos. Use IDs qualificados para specs. Várias specs independentes podem coexistir no mesmo escopo; antes de editar, reserve a spec e registre a posse dos arquivos conforme o contrato de equipe da skill. Consolide índices e estado compartilhado serialmente.

---
_Documentação centaur — schema `5.2.0`, gerada em `2026-10-29`. Atualize com `/centaur-driven-update`._
