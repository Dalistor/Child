# Como contribuir

O Child é uma base: a ideia é que você clone, treine o seu e, se quiser, devolva melhorias. Este guia resume as regras que mantêm a base confiável para todos.

## Compilar e testar

Precisa só de um compilador C++20 e do CMake (Ninja é opcional). Python 3 é usado apenas por dois testes, o dos nomes de pastas e o dos parâmetros documentados. Sem ele, esses testes são pulados.

```bash
cmake -S . -B build -G Ninja && cmake --build build
ctest --test-dir build
```

O build padrão é portátil: a mesma semente gera os mesmos pesos em qualquer máquina x86-64. `-DCHILD_NATIVE=ON` usa as instruções da sua CPU e fica mais rápido, mas os pesos deixam de ser comparáveis entre máquinas. Não use essa opção para gerar resultados que outros vão reproduzir.

Um pull request precisa compilar sem avisos (`-Wall -Wextra -Wpedantic`) e passar no `ctest`.

## Regras da base

- **Sem dependências externas**: só a biblioteca padrão do C++20. CUDA, quando vier, é um backend opcional.
- **Variáveis de ambiente**: `CHILD_NEUROGENESIS`, `CHILD_MAX_NEURONS`, `CHILD_PARAMS` e `CHILD_ENGINE` (README). Elas controlam a execução e nunca reescrevem os parâmetros gravados num engrama.
- **Aprendizado local**: o código novo aprende por regras locais e online: STDP, dopamina como terceiro fator, homeostase, regra delta no cerebelo e regras hebbianas nos segmentos. Retropropagação existe apenas no MLP de referência.
- **Determinismo**: mesma semente, mesmos pesos. Use o `Rng` do projeto, não `<random>`, para pesos. O teste `reference weights` fixa o hash dos pesos; se a sua mudança o altera de propósito, explique por quê no pull request.
- **Engrama**: a escrita é sempre atômica (`.tmp` + `rename`). Uma peça nova do cérebro grava seu estado pelo próprio `persist()`. Se o formato da seção `brain` mudar, a versão dessa seção sobe.
- **Parâmetros**: todo parâmetro novo entra no registro (`src/brain/parameters.cpp`) com unidade, faixa e fonte, e na tabela da seção 7 de `docs/parametros.md`. O teste `parameters_doc` confere.
- **Nomes do cérebro**: cada estrutura nova fica em `src/<estrutura>/` com o arquivo principal `<estrutura>.hpp`/`.cpp` (por exemplo, `src/neuron/neuron.hpp`). O `ctest` verifica.
- **Testes**: só para uma falha concreta num ponto vital ainda sem proteção. Prefira ampliar um caso existente em `tests/tests.cpp` (núcleo e MLP) ou em `tests/brain_tests.cpp` (o cérebro integrado).

## Idioma

Código, identificadores e comentários em inglês; documentação e discussão em português.

## Contratos e evidências

O comportamento esperado está em contratos versionados (`.centaur/contracts/<id>/vNNN.json`), com regras e critérios de aceite. A realização fica em `.centaur/state/`, e as provas, em `.centaur/evidence/`. Para propor um comportamento novo:

1. Abra uma issue descrevendo o que muda e como saber que funciona.
2. Se for aceito, o contrato ganha uma versão nova. Contratos aprovados não são reescritos.
3. Implemente e anexe a evidência: o comando que roda e o que ele mostra.

A pesquisa por trás das decisões está em [docs/](docs/README.md); comece pela [síntese](docs/sintese.md). Pesquisa nova vira arquivo novo ali, com fontes e data.

## Licença

Ao contribuir, você concorda que sua contribuição seja distribuída sob a [licença MIT](LICENSE) do projeto.
