# MAWI Studio

Primeira versão de um editor 3D Linux-first, pequeno e executável, inspirado no fluxo do Roblox Studio. O formato `.mawi` é próprio e não é compatível com arquivos do Roblox.

## Estado atual

- Janela X11 real com contexto GLX e renderização OpenGL de pipeline fixa.
- Viewport com grade, eixos, cubo inicial e câmera orbital (botão direito + roda do mouse).
- Explorer conectado ao modelo de cena e seleção do cubo no viewport.
- Interface Dear ImGui com docking: viewport central, Explorer, Properties e Console redimensionáveis.
- Paleta cinza/grafite, widgets nativos personalizados e fonte DejaVu Sans 14 px.
- Ferramentas Select, Move, Rotate e Scale atuam por arraste no objeto selecionado.
- Salvar o projeto atual com `Ctrl+S` e abrir o seletor de projetos com `Ctrl+O`.
- Formato de projeto versionado e validação de arquivos inválidos sem encerrar o processo.
- Tela inicial com criação/abertura de projetos, projetos recentes e documentação integrada.

## Dependências

O build usa C++17, GCC, CMake 3.16+, X11, GLX e libepoxy. Dear ImGui 1.91.9b docking está incluído em `vendor/imgui` sob licença MIT, com backend OpenGL 2. A fonte é carregada de `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf` (DejaVu Fonts License); se ausente, ImGui usa sua fonte padrão. No Ubuntu/Mint, os pacotes de sistema necessários são `libx11-dev`, `libgl-dev`, `libglx-dev`, `libepoxy-dev`, `cmake` e `g++`; a fonte pode ser obtida com `fonts-dejavu-core`. O backend não exige OpenGL 3.3 Core.

## Build e execução

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j1
./build/mawi_studio
```

Execute dentro da sessão gráfica X11 do Linux Mint. `Esc` encerra; `Ctrl+S` salva; `Ctrl+O` abre o diálogo de projetos. Arraste com botão direito sobre o viewport para orbitar, use a roda para zoom, clique para selecionar e arraste com Move/Rotate/Scale para editar.

## Projetos e ajuda

Na tela inicial, use **New Project** para informar nome e pasta de destino existente; **Choose folder** abre o seletor de diretórios do Zenity. **Open Project** aceita uma pasta que contenha `project.meta` e `scenes/main.mawi`. O projeto criado contém `project.meta` (manifesto `MAWI_WORKSPACE 1`, nome e data real de criação), `assets/` e `scenes/main.mawi` (cena versionada). Salvar escreve nessa cena. **File > Home** retorna à tela inicial; **Help & Documentation** abre o guia dentro do aplicativo, também disponível no menu **Help** do editor.

Os recentes ficam em `$XDG_CONFIG_HOME/mawi-studio/recent.txt` ou `~/.config/mawi-studio/recent.txt` e guardam caminho e última abertura; nome e criação são lidos do manifesto. **Remove from list** retira apenas a entrada, sem apagar o projeto. Não há importador 3D nesta versão, portanto **Import 3D Model** está desativado. O seletor de pastas usa o programa `zenity` quando instalado; o caminho também pode ser digitado manualmente.

## Estrutura

- `src/main.cpp`: janela, eventos e ciclo principal;
- `src/renderer.cpp`: viewport OpenGL e cena 3D;
- `src/scene.cpp`: modelo mínimo da cena;
- `src/project.cpp`: persistência `.mawi` versão 1;
- `src/workspace.cpp`: manifestos e projetos recentes;
- `src/home.cpp`: tela inicial e documentação integrada;
- `vendor/imgui`: fontes oficiais Dear ImGui e backend OpenGL 2;
- `include/`: interfaces dos módulos.

## Validação e limitações

O build usa um job (`-j1`). A tela inicial, documentação, criação e reabertura de projeto e o viewport foram verificados visualmente nesta sessão X11. A compatibilidade com outras instalações do Linux Mint ainda deve ser confirmada no computador alvo.

Teste do formato e dos recentes: `ctest --test-dir build --output-on-failure`.

Há apenas um objeto inicial, não há undo/redo nem lógica de jogo; Play/Stop e Undo/Redo ficam desativados. O salvamento usa `scenes/main.mawi` dentro do projeto aberto.
