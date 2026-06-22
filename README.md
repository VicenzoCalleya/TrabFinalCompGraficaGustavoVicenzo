# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS

Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle:

https://moodle.ufrgs.br/mod/assign/view.php?id=6018620

A aplicação desenvolvida:

Temos um humano, algumas árvores, algumas gramas altas, e alguns pegamons. O humano é nosso treinador pegamon, visto em terceira pessoa. Seu movimento é controlado pelo usuário, tal como seu modo mira. Em seu modo mira, tem a câmera modificada, e pode arremessar uma pegabola, segurando e soltando o botão direito do mouse. Com a força do arremesso ficando maior de maneira proporcional ao tempo segurando o botão antes de soltar. Se a pegabola acerta o chão, ela se choca com ele, e some após alguns segundos. Se a pegabola acerta um pegamon, temos uma captura. A captura consiste no pegamon indo para a pegabola (use sua imaginação), e então a pegabola no chão emitindo luz, até desaparecer. Todos os objetos (com excessão da grama) tem sombras, seja da luz de cima, ou da luz das capturas de pegamons. Todos os objetos (com excessão da grama) tem colisão. Os pegamons se movem em animações simples, em curva, aguardando para continuar seu caminho se algo interrompê-los. A iluminação é difusa, com sombras pretas e "duras". Na captura, a pegabola cancela as sombras geradas pela luz de cima, e gera suas proprias sombras nos objetos em um raio próximo. As animações dos pegamons se movendo são curvas de Bézier. Os pegamons, o jogador e as árvores, tem suas respectivas texturas. A pegabola arremessada, viaja em função do tempo. 

Contribuições:

Eu, Gustavo, fiquei responsável por encontrar modelos na internet que satisfazem nossas necessidades, tal como implementá-los. Fiquei responsável também por implementar a iluminação de cima e a iluminação da captura. E sou o responsável por escrever esse relatório. Já meu colega, Vicenzo, foi o responsável pelas animações dos pegamons, pela animação e implementação da pegabola no ar e da captura dos pegamons, e também pelas colisões entre objetos.

Sobre o uso de IA:

Usei Gemini e o Claude integrado ao Visual Studio Code durante a totalidade da minha parte no projeto, fazendo meus próprios ajustes e correções. Achei muito útil o uso dessas ferramentas. Tive que ativamente sugerir ideias de como fazer, para além de simplesmente pedir que seja feito, então ao menos os agentes que usei, certamente não seriam tão úteis para um usuário leigo. Também me vi corrigindo bugs durante boa parte do projeto, o que apesar de provavelmente não ter me consumido mais tempo do que me consumiria fazer o código do zero, foi um bom bocado de tempo. Também vale dizer que durante todo o processo, os modelos sugeriam mudanças sem sentido, ou que quebravam o código, o que me obrigava a corrigir o que os modelos diziam ou implementar e ver só depois o que tinha de errado.

* espaço para o Vicenzo deixar seu relato *

Imagens:

<img width="550" height="622" alt="image" src="https://github.com/user-attachments/assets/fc34350b-d02d-4d40-a84e-e1a28cbc1efb" />
<img width="947" height="515" alt="image" src="https://github.com/user-attachments/assets/02406699-6278-44f0-8013-bfa6f8a8467c" />
<img width="758" height="477" alt="image" src="https://github.com/user-attachments/assets/b8ad537c-e24e-42a1-864d-9445d263ab05" />

Manual:

Você move o personagem com "WASD". Move a câmera com o botão esquerdo do mouse e o scroll. Entra no modo de mira apertando a letra  "E" e nesse modo pode segurar o botão direito do mouse e soltar quando achar que vai ser um arremesso com força o suficiente.

Como compilar (usando a explicação do seu arquivo do Lab 2):

# Linux

Para compilar e executar este projeto no Linux, primeiro você precisa
instalar as bibliotecas necessárias (nos computadores dos laboratórios
do INF, estas bibliotecas já estão instaladas, então você pode pular
estes passos). Para tanto, execute o comando abaixo em um terminal.
Esse é normalmente suficiente em uma instalação de Linux Ubuntu:

```bash
sudo apt-get install build-essential make libx11-dev libxrandr-dev \
                     libxinerama-dev libxcursor-dev libxcb1-dev libxext-dev \
                     libxrender-dev libxfixes-dev libxau-dev libxdmcp-dev
```

Se você usa Linux Mint, talvez seja necessário instalar mais algumas bibliotecas:

```bash
sudo apt-get install libmesa-dev libxxf86vm-dev
```

Após a instalação das bibliotecas acima, você possui duas opções para compilação:

## Linux com CMake (melhor opção)

Abra um terminal, navegue até a pasta onde está este código fonte, e execute
o seguinte comando:

```bash
cmake --workflow --preset configure-build-run
```

isso é equivalente aos comandos:

```bash
cmake -B build -S .          # Cria e configura diretório de build
cmake --build build          # Faz a compilação
cmake --build build -- run   # Executa o código compilado
```

> [!TIP]
> Esta é a melhor opção, pois o CMake gera automaticamente uma
> configuração que reutiliza arquivos previamente compilados, e
> recompila somente os arquivos que foram modificados.

Também é possível remover todos arquivos compilados com:

```bash
cmake --build build -- clean # Remove arquivos compilados
cmake --build build -- run   # Re-compila e executa
```

## Linux com Makefile

Abra um terminal, navegue até a pasta onde está este código fonte, e execute
o comando "make" para compilar. Para executar o código compilado, execute o
comando "make run".

## Linux com VSCode

1) Instale o VSCode seguindo as instruções em https://code.visualstudio.com/ .

2) Instale as extensões "ms-vscode.cpptools" e "ms-vscode.cmake-tools"
no VSCode. Se você abrir o diretório deste projeto no VSCode,
automaticamente será sugerida a instalação destas extensões (pois
estão listadas no arquivo ".vscode/extensions.json").

3) Clique no botão de "Play" *NA BARRA INFERIOR* do VSCode para compilar
e executar o projeto. Na primeira compilação, a extensão do CMake para
o VSCode irá perguntar qual compilador você quer utilizar. Selecione
da lista o compilador que você deseja utilizar.

Veja mais instruções de uso do CMake no VSCode em:

https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/README.md

# macOS
Para compilar e executar esse projeto no macOS, primeiro você precisa instalar o
HOMEBREW, um gerenciador de pacotes para facilitar a instação de bibliotecas. O
HOMEBREW pode ser instalado com o seguinte comando no terminal:

```zsh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Após a instalação do HOMEBREW, a biblioteca GLFW deve ser instalada. Isso pode
ser feito pelo terminal com o comando:

```zsh
brew install glfw
```

## macOS com Makefile
Abra um terminal, navegue até a pasta onde está este código fonte, e execute
o comando "make -f Makefile.macOS" para compilar. Para executar o código
compilado, execute o comando "make -f Makefile.macOS run".

## macOS com CMake
Você utiliza macOS e conseguiu utilizar o CMake para compilar este projeto?
Então mande um e-mail para o professor em <eslgastal@inf.ufrgs.br> com
as alterações necessárias no CMakeLists.txt e uma lista de instruções
para colocar neste LEIAME.txt .

# Windows

## Windows com VSCode (Visual Studio Code)

1) Instale o VSCode seguindo as instruções em https://code.visualstudio.com/ .

2) Instale o compilador GCC no Windows seguindo as instruções em
https://code.visualstudio.com/docs/cpp/config-mingw#_installing-the-mingww64-toolchain .

Alternativamente, se você já possui o Code::Blocks instalado no seu PC
(versão que inclui o MinGW), você pode utilizar o GCC que vem com esta
instalação no passo 5.

3) Instale o CMake seguindo as instruções em https://cmake.org/download/ .
Alternativamente, você pode utilizar algum package manager do
Windows para fazer esta instalação, como https://chocolatey.org/ .

4) Instale as extensões "ms-vscode.cpptools" e "ms-vscode.cmake-tools"
no VSCode. Se você abrir o diretório deste projeto no VSCode,
automaticamente será sugerida a instalação destas extensões (pois
estão listadas no arquivo ".vscode/extensions.json").

5) Abra as configurações da extensão cmake-tools (Ctrl-Shift-P e
busque por "CMake: Open CMake Tools Extension Settings"), e adicione o
caminho de instalação do GCC na opção de configuração "additionalCompilerSearchDirs".

Por exemplo, se você quiser utilizar o compilador MinGW que vem junto
com o Code::Blocks, pode preencher o diretório como
"C:\Program Files\CodeBlocks\MinGW\bin" (verifique se este é o local
de instalação do seu Code::Blocks).

6) Clique no botão de "Play" na barra inferior do VSCode para compilar
e executar o projeto. Na primeira compilação, a extensão do CMake para
o VSCode irá perguntar qual compilador você quer utilizar. Selecione
da lista o compilador GCC que você instalou com o MSYS/MinGW.

Veja mais instruções de uso do CMake no VSCode em:

https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/README.md

## Soluções de Problemas

Caso você tenha problemas de compilação no Windows com GCC, cuide para
extrair o código do laboratório em um caminho que não contenha espaços
no nome de algum diretório:

- Caminho OK: `C:\Users\JohnDoe\Documents\CGVis\Lab1`

- Caminho NÃO OK: `C:\Users\JohnDoe\Documents\Fundamentos de CG\Lab1`

Caso você tenha problemas em executar o código deste projeto, tente atualizar o
driver da sua placa de vídeo.
