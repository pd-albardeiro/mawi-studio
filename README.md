# MAWI Studio

Primeira versão de um editor 3D Linux-first, pequeno e executável, inspirado no fluxo do Roblox Studio. O formato `.mawi` é próprio e não é compatível com arquivos do Roblox.

## Estado atual

- Janela X11 real com contexto GLX e renderização OpenGL de pipeline fixa.
- Viewport com grade, eixos, cubo inicial e câmera orbital (botão direito + roda do mouse).
- Explorer conectado ao modelo de cena e seleção do cubo no viewport.
- Interface Dear ImGui com docking: viewport central, Explorer, Properties e Console redimensionáveis.
- Paleta cinza/grafite, widgets nativos personalizados e fonte DejaVu Sans 14 px.
- Ferramentas Select, Move, Rotate e Scale atuam por arraste no objeto selecionado.
- Salvar e abrir em `mawi_project.mawi` com `Ctrl+S` e `Ctrl+O`.
- Formato de projeto versionado e validação de arquivos inválidos sem encerrar o processo.

## Dependências

O build usa C++17, GCC, CMake 3.16+, X11, GLX e libepoxy. Dear ImGui 1.91.9b docking está incluído em `vendor/imgui` sob licença MIT, com backend OpenGL 2. A fonte é carregada de `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf` (DejaVu Fonts License); se ausente, ImGui usa sua fonte padrão. No Ubuntu/Mint, os pacotes de sistema necessários são `libx11-dev`, `libgl-dev`, `libglx-dev`, `libepoxy-dev`, `cmake` e `g++`; a fonte pode ser obtida com `fonts-dejavu-core`. O backend não exige OpenGL 3.3 Core.

## Build e execução

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j1
./build/mawi_studio
```

Execute dentro da sessão gráfica X11 do Linux Mint. `Esc` encerra; `Ctrl+S` salva; `Ctrl+O` abre o arquivo padrão. Arraste com botão direito sobre o viewport para orbitar, use a roda para zoom, clique para selecionar e arraste com Move/Rotate/Scale para editar.

## Estrutura

- `src/main.cpp`: janela, eventos e ciclo principal;
- `src/renderer.cpp`: viewport OpenGL e cena 3D;
- `src/scene.cpp`: modelo mínimo da cena;
- `src/project.cpp`: persistência `.mawi` versão 1;
- `vendor/imgui`: fontes oficiais Dear ImGui e backend OpenGL 2;
- `include/`: interfaces dos módulos.

## Validação e limitações

O build usa um job (`-j1`). O contexto OpenGL e a interação visual devem ser validados na sessão X11 alvo.

Há apenas um objeto inicial, não há undo/redo nem lógica de jogo; Play/Stop e Undo/Redo ficam desativados. O salvamento usa o nome fixo `mawi_project.mawi`.
