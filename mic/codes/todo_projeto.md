# TODO List - Projeto 1 (EEL7030)

## Funcionalidades Pendentes
- [x] Implementar lógica para resetar o cronômetro corretamente no botão de reset.
- [|] Adicionar tratamento de erros para inicialização do display LCD.
- [x] Garantir que o temporizador funcione corretamente em diferentes resoluções.
- [x] Testar a funcionalidade de marcação de volta (case 2 na interrupção).
- [x] Retirar o efeito debouncing dos botões utilizando temporizadores de software `(OBRIGATORIO)`.

## Melhorias no Código
- [x] Refatorar a função `loop` para melhorar a legibilidade.
- [|] Substituir o `delay` por uma abordagem baseada em temporizadores para evitar bloqueios.
- [x] Adicionar comentários explicativos em trechos críticos do código `(OBRIGATORIO)`.

## Testes
- [|] Criar testes unitários para a função `interrupcao`.
- [x] Testar o comportamento do sistema com diferentes valores de `tempo_alarme`.
- [x] Validar a exibição no display LCD com diferentes formatos de tempo.

## Documentação
- [x] Escrever documentação para a configuração do hardware (pinos, display, etc.) `(OBRIGATORIO)`. 
- [x] Adicionar explicação sobre o funcionamento do temporizador e interrupções `(OBRIGATORIO)`.
- [|] Criar um diagrama de fluxo para o funcionamento do cronômetro.

## Outros
- [|] Verificar compatibilidade com outras versões do ESP-IDF.
- [ ] Realizar testes finais no hardware real.
