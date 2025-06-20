# Jogo da Memoria

Este projeto traz para o terminal um jogo onde o desafio é encontrar todos os pares de cartas no menor número de tentativas possível.

## Regras:

- O tabuleiro tem 16 cartas (8 pares), embaralhadas aleatoriamente.
- Em cada rodada, o jogador escolhe duas cartas informando suas coordenadas (linha e coluna).
- Se as cartas forem iguais, o jogador ganha 10 pontos e o par permanece aberto.
- Se forem diferentes, o jogador perde 1 ponto e uma tentativa.
- O jogador tem um limite de 15 tentativas para errar pares.
- O jogo termina quando todos os pares forem encontrados (vitoria) ou quando as tentativas acabarem (derrota).
- É possível digitar `sair` ou `q` a qualquer momento para encerrar o jogo e salvar o progresso.

## Como compilar e rodar

1. Abra o terminal na pasta do projeto.
2. Compile o programa com:
```
   
   gcc jogo_da_Memoria.c -o jogo
   ```
   
3. Execute o jogo:
```
   
   ./jogo.exe
   ```

## Organização do código

  - Estruturas principais: Carta, Jogador, JogoMemoria.
  - Funcoes importantes:
    - inicializarJogo: Prepara o tabuleiro e adiciona o jogador.
    - exibirTabuleiro: Mostra o tabuleiro na tela.
    - jogarJogoMemoria: Controla o fluxo do jogo.
    - salvarPlacar: Salva o resultado do jogo no arquivo placar.txt e protege contra alteracoes manuais.
    - exibirPlacar: Mostra o histórico de partidas e avisa se o arquivo foi adulterado.
    - lerCoordenadasOuSair: Permite informar coordenadas ou sair do jogo.
    - Outras funções para validação, liberar memória e controlar o andamento do jogo.

- **placar.txt**: Arquivo gerado automaticamente para registrar o histórico das partidas, pontuação e tentativas restantes. O arquivo tem um sistema de verificação para detectar alterações manuais.