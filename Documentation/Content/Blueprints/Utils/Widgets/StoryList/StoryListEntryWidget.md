# Widget StoryListEntryWidget

Widget de um elemento da lista de histórias da cena. Contém um dropdown para a classe da história, outro para a tag de âncora associada, um botão para abrir o widget MessageEditor para alterar as mensagens de erro da história, um botão de expansão para exibir os passos da história e um botão para remoção do passo.

## Graph

### Variáveis

#### IsInitializing

Variável utilizada para indicar se alterações de valores nos dropdowns são referentes ao procedimento de inicialização do item, ao passar os valores obtidos com o objeto de entrada. Dessa forma, evita que essas alterações triggem as rotinas de tratamento para alterações feitas pelo desenvolvedor no editor widget StoryBuilder durante a inicialização.

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato MessageListEntryObject dentro do editor widget StoryBuilder. 
    
Define o conteúdo do botão de expansão como "+" e o index do AnswerContent Widget Switcher para 0, exibindo o estado colapsado das respostas da questão. Configura o valor da variável IsInitializing para *true*. Configura a variável StoryListEntry do widget para referenciar o objeto de entrada, chama as duas funções para população dos dropdowns e atualiza o valor da variável IsInitializing para *false*.

#### OnSelectionChanged (StoryClass)

Caso a alteração não corresponda à inicialização do item, chama o método UpdateStoryClass para atualizar a variável StoryListEntry e chama o método OnApplyStoryChanges para acionar a interface StoryListManagerInterface e salvar as alterações.

#### OnSelectionChanged (StoryAnchorTag)

Caso a alteração não corresponda à inicialização do item, chama o método UpdateStoryAnchor para atualizar a variável StoryListEntry e chama o método OnApplyStoryChanges para acionar a interface StoryListManagerInterface e salvar as alterações.

#### OnClicked (RemoveStoryButton)

Após interagir com o botão de remoção do item, chama a rotina OnDeleteStoryButtonClicked, responsável por acionar a interface StoryListManagerInterface para remover a história.

#### OnClicked (ExpandStoryButton)

Após interagir com o botão de expandir ou colapsar o conteúdo das respostas da questão, aciona a rotina ToggleStepContent que gerencia as mudanças de estado da visibilidade dos passos da história.

#### OnClicked (RefreshStepButton)

Após interagir com o botão de atualizar a lista de respostas, chama o método GetStoryStepsArray, responsável por acionar a interface StoryListManagerInterface e, em seguida, o método UpdateStepList para sincronizar a lista de passos com a nova busca de passos.

#### OnClicked (AddStepButton)

Após interagir com o botão de adicionar uma resposta à questão, chama o método OnAddStepClicked, responsável por acionar a interface StoryListManagerInterface e tratar a inclusão de um novo passo na lista.

#### OnClicked (EditErrorMessagesButton)

Após interagir com o botão do MessageEditor, aciona a interface StoryListManagerInterface e chama o método OpenMessageEditorForStory para abrir o widget e exibir os recursos de edição das mensagens de erro da história.

### Funções

#### FillStoryClassesDropdown

Rotina que popula o dropdown de classes da história.

Inicia com a limpeza dos itens do dropdown, aciona a interface StoryListManagerInterface e chama o método GetStoriesClassesMapKeys, obtendo a lista de classes disponíveis para as histórias. Então, varre a lista, adicionando um item no dropdown para cada elemento. Finalmente, verifica se o valor recebido de classe do objeto de entrada existe na lista, se for o caso, deixa esse valor como o item selecionado. Do contrário, deixa a primeira entrada que corresponde à classe base BP_StoryBase.

#### FillAnchorNamesDropdown

Rotina que popula o dropdown de tags de âncoras registradas.

Inicia com a limpeza dos itens do dropdown, aciona a interface StoryListManagerInterface e chama o método GetAnchorsNamesMapKeys, obtendo a lista de tags de âncoras registradas . Então, varre a lista, adicionando um item no dropdown para cada elemento. Finalmente, verifica se o valor recebido de tag do objeto de entrada existe na lista, se for o caso, deixa esse valor como o item selecionado. Do contrário, deixa a primeira entrada que corresponde a uma mensagem de nenhuma tag selecionada.

#### OnApplyStoryChanges

Aciona a interface StoryListManagerInterface e chama o método OnStoryChanged e passa todas as propriedades do valor atual da variável StoryListEntry.

#### OnDeleteStoryButtonClicked

Aciona a interface StoryListManagerInterface e chama o método OnStoryDeleted e passa os indexes obtidos com o objeto de entrada.

#### UpdateStoryClass

Constrói um novo objeto da classe StoryListEntryObject contendo o novo valor de classe e substitui a referência da variável StoryListEntry. Por fim, chama o método StoryChanged para acionar a interface StoryListManagerInterface e salvar as alterações.

#### UpdateStoryAnchor

Constrói um novo objeto da classe StoryListEntryObject contendo o novo valor de tag de âncora e substitui a referência da variável StoryListEntry. Por fim, chama o método StoryChanged para acionar a interface StoryListManagerInterface e salvar as alterações.

#### ToggleStepContent

A partir do valor atual do Widget Switcher, troca para ou 0 ou 1, habilitando um container vazio ou a lista de passos respectivamente. Além disso, atualiza o texto do botão de expansão para exibir "+" se os passos estiverem colapsados e "-" no caso contrário. Finalmente, após habilitar a lista de passos com o Widget Switcher, chama a função GetStoryStepsArray para buscar os passos com a interface StoryListManagerInterface e chama a função UpdateStepList para atualizar a lista de passos exibidos.

#### UpdateStepList

Rotina para atualizar a lista de passos exibidos. 

Inicia com a limpeza da lista, seguida por um loop varrendo os passos recebidos e, a cada iteração, cria um objeto da classe StepListEntryObject contendo os valores obtidos com a chamada da interface e insere no widget de lista de passos, acionando o seu método OnListObjectSet e populando a lista.

#### GetStoryStepsArray

Aciona a interface StoryListManagerInterface e chama o método GetStorySteps para obter os passos associados à história do item. 


#### OnAddStepClicked

Aciona a interface DialogueListManagerInterface para chamar o método AddStoryStep e, em seguida, o método UpdateStepList para sincronizar a lista de respostas com a adição.