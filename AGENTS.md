# Child

## Visão Geral
Base virgem de uma rede neural capaz de aprender, escrita em C++20 sem dependências externas. Tem três camadas: **captação** (texto/código → bytes), **processamento** (rede que prevê o próximo byte e aprende) e **armazenamento** (checkpoint com linhagem, que permite ramificar). O objetivo de longo prazo é aproximar o aprendizado do humano; este repositório cobre só a base. Detalhes: [arquitetura](.centaur/system/architecture.md).

## Recuperação de contexto
Priorize busca e símbolos do editor. Graphify é opcional para relações amplas; consulte `.centaur/graphify/graph.json` pelo adaptador da skill apenas quando útil. Confirme fontes atuais. Não indexe automaticamente após mudanças. Decisões e documentação gerada ficam em `.centaur/`; código e testes comprovam comportamento.

## Memória de decisões e mudanças
Backend `files` (confirmado em `.centaur/workspace.json`): registros em `.centaur/implements/` com índice em [status.md](.centaur/implements/status.md). Chame a skill `centaur-driven-memory` para consultar ou registrar. Memória histórica não substitui código, testes, regras ou o estado das specs.

## Contratos e realização
Contratos em `.centaur/contracts/<id>/vNNN.json` definem o comportamento desejado e a autonomia. Estado em `.centaur/state/` e evidências em `.centaur/evidence/` registram a realização; o ciclo normativo de `centaur-driven-graphify` define formatos e gates. Contratos aprovados são versionados, não reescritos. Antes de editar, consultar apenas as regras e limites pertinentes. Checklist de spec não comprova comportamento. Consulte specs, fontes e evidências diretamente no terminal. Apresente resultado, validação e próxima ação com links; detalhes ficam nos registros.

Contrato vigente: [`base`](.centaur/contracts/base/v002.json), aprovado em 2026-10-29 (a v001 foi substituída para retirar decisões de evolução futura, que bloqueavam as regras). Estado em [base.json](.centaur/state/base.json); as evidências são testes reais ligados ao hash da v002. Decisões futuras em aberto: [arquitetura](.centaur/system/architecture.md).

## Comandos essenciais
```bash
cmake -S . -B build -G Ninja && cmake --build build   # compilar (Release, -O3 -march=native)
./build/child_tests                                   # testes (também: ctest --test-dir build)
./build/child init root.child --name root             # base virgem
./build/child fork root.child a.child --name a        # ramificar
./build/child feed a.child src --epochs 3             # captar e aprender
./build/child info a.child                            # linhagem e estado
./build/child sample a.child --prompt "int "          # gerar texto
python3 <skills>/centaur-driven-graphify/scripts/validate-lifecycle.py .   # validar registros Centaur
```
Rode os comandos a partir da raiz do projeto. Arquivos `.child` são checkpoints; não os versione.

## Arquitetura de Camadas
| Camada | Pasta | Responsabilidade | Proibido |
|---|---|---|---|
| Captação | `src/capture/` | Fonte → bytes | Conhecer o modelo ou o armazenamento |
| Processamento | `src/model/` | Rede, gradientes, `learn` | E/S de arquivos; conhecer captação ou armazenamento |
| Armazenamento | `src/store/` | Checkpoint, linhagem, `fork` | Alterar pesos; depender de algo além de `Config` do modelo |
| CLI | `src/main.cpp` | Orquestra as camadas | Lógica de aprendizado ou de formato |

Dependências apontam para dentro: só o CLI conhece as três camadas. Mudar o formato do checkpoint exige subir `kVersion` em `store.cpp`.

## Testes
Harness próprio em [tests/tests.cpp](tests/tests.cpp), executável `child_tests`, registrado no `ctest`. Nenhuma meta de cobertura. Pontos vitais com proteção atual: gradientes da retropropagação contra diferença numérica, aprendizado de um padrão, integridade e atomicidade do checkpoint, isolamento entre ramificações, captação determinística. Teste novo só para falha concreta em ponto vital ainda sem proteção; prefira atualizar um caso existente. Mudanças de baixo impacto: compilar e rodar `child_tests`.

## Vocabulário e Idioma do Código
Código, identificadores e comentários em inglês; documentação e conversa em português. Termos: *virgin network* = base virgem (pesos aleatórios, nada aprendido); *fork* = ramificação; *lineage* = linhagem (id, pai, nome, bytes aprendidos); *capture* = captação; *window/context* = janela de bytes anteriores.

## Regras e Restrições
- Sem bibliotecas externas: só a biblioteca padrão do C++20.
- Compilar sem avisos com `-Wall -Wextra -Wpedantic`.
- Preserve o determinismo: mesma semente, mesmos pesos (usa `Rng` próprio, não `<random>` para pesos).
- Escrita de checkpoint é atômica (`.tmp` + `rename`); nunca escreva direto no destino.
Carregue `clean-code` e as referências pertinentes ao planejar, implementar ou revisar código. Preserve o modo TDD/direto e validação proporcional definidos pelo Centaur. As instruções do usuário e deste projeto prevalecem. Consulte `.clean/` se existir, sem escrevê-lo nestes fluxos.

## Módulos e Equipe
Modo **individual**: o desenvolvedor acumula responsabilidade e integração. Escopo único `master`.
Consulte `.centaur/workspace.json` para escopos, caminhos e responsáveis. Specs ficam em `.centaur/specs`; implementações seguem o backend de memória, preservando os diretórios legados; leia seus índices sob demanda e confirme status nos READMEs canônicos. Use IDs qualificados para specs. Várias specs independentes podem coexistir no mesmo escopo; antes de editar, reserve a spec e registre a posse dos arquivos conforme o contrato de equipe da skill. Consolide índices e estado compartilhado serialmente.

---
_Documentação centaur — schema `5.2.0`, gerada em `2026-10-29`. Atualize com `/centaur-driven-update`._
