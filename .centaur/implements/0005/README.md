# [0005] Repositório pronto para ser clonado

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** direto
**Spec:** `.centaur/specs/0006/` — Task 01

## Solicitação
"pode executar todas" (ponto 5: licença, guia de contribuição e exemplo de ponta a ponta).

## O que foi feito
- `LICENSE` MIT (Copyright 2026 Dalistor).
- `CONTRIBUTING.md`: compilar e testar, regras da base, idioma, contratos e evidências, licença das contribuições.
- README: seção "Do clone ao primeiro fork treinado" (substitui "Como usar a base atual"), links para o guia e a licença.

## Arquivos criados
- `LICENSE`, `CONTRIBUTING.md`

## Arquivos modificados
- `README.md`

## Decisões técnicas
- O exemplo usa `--seed 1` explícito para reforçar a reprodutibilidade.
- O ABR-03 foi verificado numa cópia limpa da árvore candidata, porque as mudanças ainda não estão no remoto. Depois do push, repetir com `git clone` real.

## Como validar
Executar o bloco do README num diretório novo.

## Contrato e realização
`base-aberta@1` ABR-01 (ev-1d1961e5e33646098f748a400c3f5742), ABR-02 (ev-496bcdc3a7a84d45815539516a6cbec1), ABR-03 (ev-ebf769a3e31c44a5b874e4bcb2d3986f). Implementadas, verificadas, entrega local.

## Resultado da validação
Exemplo completo: compila, `ctest` passa, `meu.child` aprendeu 73.332 bytes, `root.child` ficou com 0.
