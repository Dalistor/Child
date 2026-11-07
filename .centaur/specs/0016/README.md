# 2026 Parâmetros lidos de arquivo, com faixa e fonte (CFG-01, PAR-01)

**Data:** 2026-11-07
**Status:** Em revisão
**Escopo:** master
**Responsável:** Dalistor
**Contrato:** cerebro@2
**Regras:** CFG-01, PAR-01
**Spec mestre:** master/0012
**Specs filhas:** —
**Dependências:** —
**Solicitação original:** "Pode trabalhar no projeto até que a base esteja totalmente concluída e funcional, precisa ser capaz de expandir e criar neurônios virtuais se necessário (Deve ter uma váriavel de ambiente para esta e outras versões necessárias)" (/goal de 2026-11-07)

## Objetivo e demonstração
- **CFG-01:**
  - `child init --params FILE` (ou `CHILD_PARAMS`) lê linhas `nome = valor` e grava os parâmetros no engrama;
  - um valor ausente usa o padrão;
  - um valor desconhecido, malformado ou fora da faixa falha, nomeando o parâmetro;
  - o fork herda os parâmetros;
  - o mesmo arquivo e a mesma semente dão a mesma seção `brain`.
- **PAR-01:** os 64 parâmetros do registro aparecem em `docs/parametros.md` §7 com unidade, padrão e faixa, conferidos por `tests/parameters_doc.py`.

## Limite
O engrama inteiro de duas bases difere pelo id da linhagem, que é a identidade de cada base, por desenho. A seção `brain` é idêntica.


## Checklist de conclusão
- [x] Task 01 — Registro, leitura e descrição dos parâmetros (CFG-01)
- [x] Task 02 — Tabela e conferência das faixas (PAR-01)

## Execução
/centaur-driven-run master/0016

## Estado
O ciclo normativo define o estado. Checkboxes não concluem a spec: exigir aceite, prova corrente e integração.
