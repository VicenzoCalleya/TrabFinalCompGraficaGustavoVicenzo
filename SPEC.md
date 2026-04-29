# Especificação da Implementação

> [!CAUTION]
> - Você <ins>**não pode utilizar ferramentas de IA para escrever esta
>   especificação**</ins>

## Integrantes da dupla

- **Aluno 1 - Nome**: Vicenzo Lamberti Calleya
- **Aluno 1 - Cartão UFRGS**: 589370

- **Aluno 2 - Nome**: Gustavo Azevedo da Silveira
- **Aluno 2 - Cartão UFRGS**: 588886

## Detalhes do que será implementado

- **Título do trabalho**: Pegamon Legends: Arceus
- **Parágrafo curto descrevendo o que será implementado**: Iremos fazer uma cópia básica do famoso jogo "Pokémon Legends: Arceus", ao invés de nos concentrarmos na parte de batalha porém, iremos nos focar na captura de pokémons, usando sua locomoção e furtividade para consegui-los. Essas capturas são feitas por meio de pokebolas, em que o usuário terá que arremessar nos pokémons para ter uma chance de capturá-los.

## Especificação visual

### Vídeo - Link

> [!IMPORTANT]
> - Coloque aqui um link para um vídeo que mostre a aplicação gráfica
>   de referência que você vai implementar. **Sua implementação deverá
>   ser o mais parecido possível com o que é mostrado no vídeo (mais
>   detalhes abaixo).**
> - **Você não pode escolher como referência: (1) algum trabalho realizado
>   por outros alunos desta disciplina, em semestres anteriores. (2) Minecraft.**
> - Por exemplo, você pode colocar um vídeo de um jogo que você gosta,
>   e seu trabalho final será uma re-implementação do jogo.
> - O vídeo pode ser um link para YouTube, Google Drive, ou arquivo mp4 dentro
>   do próprio repositório. Mas, garanta que qualquer um tenha
>   permissão de acesso ao vídeo através deste link.

https://youtu.be/4iG_2mQbHQg

### Vídeo - Timestamp

> [!IMPORTANT]
> - Coloque aqui um **intervalo de ~30 segundos** do vídeo acima, que
>   será a base de comparação para avaliar se o seu trabalho final
>   conseguiu ou não reproduzir a referência.

- **Timestamp inicial**: 1:36
- **Timestamp final**: 2:09

### Imagens

> [!IMPORTANT]
> - Coloque aqui **três imagens** capturadas do vídeo acima, que você
>   irá usar como ilustração para as explicações que vêm abaixo.

<img width="1360" height="768" alt="image" src="https://github.com/user-attachments/assets/90c8881e-bab4-4268-a7a1-5c4cd4b7630d" />
<img width="1360" height="768" alt="image" src="https://github.com/user-attachments/assets/3ba3f3de-343d-461c-a51c-d28fcdf00121" />
<img width="1360" height="768" alt="image" src="https://github.com/user-attachments/assets/a56e24a0-a115-45b2-8103-783cab001f83" />


## Especificação textual

Para cada um dos requisitos abaixo (detalhados no [Enunciado do Trabalho final - Moodle](https://moodle.ufrgs.br/mod/assign/view.php?id=6018620)), escreva um parágrafo **curto** explicando como este requisito será atendido, apontando itens específicos do vídeo/imagens que você incluiu acima que atendem estes requisitos.

### Malhas poligonais complexas
Será introduzido o modelo do personagem controlado pelo usuário, formado por uma malha poligonal complexa. Além disso, tanto os pokémons quanto alguns objetos do cenário (como árvores) serão formados por esse tipo de malha

### Transformações geométricas controladas pelo usuário
O usuário irá conseguir mover o personagem pelo mapa, com o personagem se rotacionando e se virando para a direção que o usuário pedir. Além disso, o usuário pode jogar uma pokebola, que embora o jogador não possa controlar totalmente sua trajetória, ele controla os parâmetros iniciais

### Diferentes tipos de câmeras
Vai existir uma câmera em que o usuário vê seu personagem em terceira pessoa. Outra que vai existir é uma câmera de mira, onde haverá zoom e um foco centralizado em onde o usuário estiver mirando.

### Instâncias de objetos
Haverá várias instâncias de objetos, principalmente relacionadas com o cenário. Múltiplos objetos com o mesmo modelo incluem: Árvores, Grama alta, alguns pokémons, etc.

### Testes de intersecção
Testes de colisão erão feitos para verificar se uma pokebola atingiu um pokemon (ou o chão, caso ele tenha errado). Além disso, também será verificado se o jogador bateu com uma árvore, ou outro objeto pelo cenário.

### Modelos de Iluminação em todos os objetos
<mark>`<preencher>`</mark>

### Mapeamento de texturas em todos os objetos
<mark>`<preencher>`</mark>

### Movimentação com curva Bézier cúbica
<mark>`<preencher>`</mark>

### Animações baseadas no tempo ($\Delta t$)
<mark>`<preencher>`</mark>

## Limitações esperadas


> [!IMPORTANT]
> - Coloque aqui uma lista de detalhes visuais ou de interação que
>   aparecem no vídeo e/ou imagens acima, mas que você **não pretende
>   implementar** ou que você **irá implementar parcialmente**.
> - Para cada item, **explique por que** não será implementado ou por
>   que será implementado parcialmente.

Não faremos a interface do jogo, pelo foco em computação gráfica. Além disso, são muitas informações que não são utilizadas, como por exemplo o tipo de pokebola que você vai utilizar.

Não faremos gráficos tão bonitos quanto os do jogo original, pois para os fins dessa atividade acreditamos que algo do nível de uma grande empresa não se
vê necessário.
