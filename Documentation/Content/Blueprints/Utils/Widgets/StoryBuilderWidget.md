# Widget StoryBuilderWidget

Widget que concentra a operação de todos os outros widgets desenvolvidos. Contém 3 abas para o AnchorsManager, o StoryTeller e o DialogueBuilder.

*** Versão atual do DialogueBuilder só opera com 1 diálogo na cena ***

## Abas do Widget

### AnchorsManager

Na aba do AnchorsManager, em primeiro lugar, há um widget switcher para trocar entre o conteúdo da aba e uma mensagem de ausência de um objeto BP_AnchorsManager na cena, que contém um botão para adicionar à cena o AnchrosManager. Quando há o objeto, fica visível um menu de edição de nomes de âncoras que contém um botão de adicionar tags à lista e um botão de recarregar a tela, seguidos pela lista de AnchorTagListEntryWidgets. 

Abaixo disso, há uma seção com um botão para recarregá-la e um dropdown a partir do qual é possível selecionar os objetos do tipo BP_Anchor presentes na cena, utilizando seus nomes. Quando uma âncora é selecionada, um widget switcher troca entre exibir um container vazio para um menu de edição da tag associada à âncora. Neste menu, há um dropdown com as tags definidas na lista acima e ao ser selecionado um elemento do dropdown, a tag da âncora é atualizada.

### StoryTeller

Na aba do StoryTeller, há também um widget switcher para trocar entre o conteúdo da aba e uma mensagem de ausência do StoryTeller na cena, contendo um botão para adicioná-lo. Quando o conteúdo está visível, há um botão para adicionar histórias e um para recarregar a seção, seguidos por uma lista de StoryListEntryWidgets.

### DialogueBuilder

Similarmente aos anteriores, há um widget switcher para trocar entre o conteúdo da aba e uma mensagem de ausência do DialogueManager na cena, contendo um botão para adicioná-lo. Quando o conteúdo da aba está visível, isto é, quando há um DialogueManager na cena, há um botão para adicionar questões de diálogo, um para recarregar a seção e, logo após, uma lista de DialogueListEntryWidgets.

## Interfaces

Contém as interfaces: 

* AnchorTagListManagerInterface
* AnswerListManagerInterface
* DialogueListManagerInterface
* StepListManagerInterface
* StoryListManagerInterface

## Detalhe de Desenvolvimento

Escolheu-se desenvolver todo o sistema em um único editor widget para evitar a necessidade de se abrir múltiplos widgets para gerenciar todos os recursos desenvolvidos. Diante disso, como há funções que não existem em widgets normais, apenas em editor widgets, houve a necessidade de concentrar uma grande parte da lógica do sistema neste widget. Assim, atingiu-se a quantidade atual de eventos e funções encontradas neste widget.

## Graph

### EventGraph

#### Construct

Na inicialização do widget, executa-se os métodos TryFindAnchorsManager, TryFindStoryTeller e TryFindDialogueManager para tentar popular as abas do StoryBuilder.

#### OnClicked (AnchorButton)

Interação com o botão da aba AnchorButton, chama a rotina de tratamento OnAnchorButtonClicked que realiza o carregamento da aba.

#### OnClicked (AddStory)

Interação com o botão AddStory da aba do StoryTeller, chama a rotina OnAddStoryClicked para inserir uma nova história na lista.

#### OnClicked (AddAnchorButton)

Interação com o botão AddAnchorButton da aba do AnchorsManager, chama a rotina OnAddAnchorClicked que trata a inserção de mais uma tag na lista de tags.

#### OnClicked (CreateAnchorsManagerInstance)

Interação com o botão CreateAnchorsManagerInstance do conteúdo do widget switcher exibido se não há um AnchorsManager na cena. Chama a rotina OnCreateAnchorManagerClicked para gerenciar a adição de uma instância da classe na cena e, logo em seguida, chama OnAnchorButtonClicked para atualizar a aba do AnchorsManagers.

#### OnClicked (CreateStoryTellerInstance)

Interação com o botão CreateStoryTellerInstance do conteúdo do widget switcher exibido se não há um StoryTeller na cena. Chama a rotina OnCreateStoryTellerClicked para gerenciar a adição de uma instância da classe na cena e, logo em seguida, chama OnStoryTellerButtonClicked para atualizar a aba do StoryTeller.

#### OnClicked (RefreshAnchorButton)

Interação com o botão de recarregar da aba do AnchorsManager, recarrega a lista de tags de âncoras através da rotina UpdateAnchorsList.

#### OnClicked (StoryTellerButton)

Interação com o botão da aba StoryTeller, chama a rotina de tratamento OnStoryTellerButtonClicked que realiza o carregamento da aba.

#### OnTagChanged

Método da originário da interface AnchorTagListManagerInterface, chama a função OnAnchorTagListItemChanged que interage com o AnchorsManager na cena e atualiza seu conteúdo.

#### OnTagDeleted

Método da originário da interface AnchorTagListManagerInterface, chama a função OnAnchorTagListItemDeleted que interage com o AnchorsManager na cena e atualiza seu conteúdo.

#### OnSelectionChanged (AnchorsInScene)

Interação com o dropdown AnchorsInScene que exibe a lista de âncoras instanciadas, pelo seu nome. Após a alteração do valor do dropdown, atualiza a aba para exibir, caso haja uma âncora selecionada, o trecho de edição de tags ou escondê-lo, caso contrário. Faz isso ao chamar o método NewAnchorSelected e repassa a opção selecionada.

#### OnClicked (RefreshStoryList)

Interação com o botão de recarregar da aba do StoryTeller, recarrega a lista de histórias exibidas através da rotina UpdateStoriesList. 

#### OnStoryChanged

Método originário da interface StoryListManagerInterface, chama a função OnStoryListItemChanged que interage com o StoryTeller na cena e atualiza o seu conteúdo.

#### OnStoryDeleted

Método originário da interface StoryListManagerInterface, chama a função OnStoryListItemDeleted que interage com o StoryTeller na cena e atualiza o seu conteúdo.

#### OnSelectionChanged (AnchorTagValue)

Interação com o dropdown de tags de uma âncora, visível após a escolha de uma âncora em cena, na aba do AnchorsManager. Chama a rotina OnApplyAnchorTagToAnchor que interage com a âncora selecionada e atualiza o valor de sua tag.

#### OnStepDeleted

Método originário da interface StepListManagerInterface, chama a função OnStepDeleted que interage com o StoryTeller na cena e atualiza o seu conteúdo.

#### OnStepChanged

Método originário da interface StepListManagerInterface, chama a função OnStepChanged que interage com o StoryTeller na cena e atualiza o seu conteúdo.

#### OnClicked (RefreshAnchorsInScene)

Interação com o botão de recarregar o dropdown de âncoras na cena, aciona o método TryFindAllAnchorsInScene para reconstruir o dropdown.

#### OpenMessageEditoForStory

Método originário da interface StoryListManagerInterface. Chama o método SpawnMessageEditor para adicionar uma instância do widget no ambiente de desenvolvimento, passa o index da história para o widget e associa seus dispatchers de GetMessages e SetMessages aos eventos OnGetStoryMessagesRequested e OnSetStoryMessagesRequested, respectivamente. Em seguida, chama o método BeginRoutines do editor.

#### OnGetStoryMessagesRequested

Evento criado para acoplar a função GetStoryMessageRequest ao dispatcher GetMessages do MessageEditor, para tratar o levantamento de mensages de erro de uma história.

#### OnSetStoryMessagesRequested

Evento criado para acoplar a função SetStoryMessageRequest ao dispatcher SetMessages do MessageEditor, para tratar a edição de mensages de erro de uma história.

#### OnGetStepMessageRequest

Método originário da interface StepListManagerInterface. Chama o método SpawnMessageEditor para adicionar uma instância do widget no ambiente de desenvolvimento, passa o index do step e o index da história (como superindex) para o widget e associa seus dispatchers de GetMessages e SetMessages aos eventos OnGetStepMessagesRequested e OnSetStepMessagesRequested, respectivamente. Em seguida, chama o método BeginRoutines do editor.

#### OnSetStepMessageRequest

Evento criado para acoplar a função GetStepMessageRequest ao dispatcher GetMessages do MessageEditor, para tratar o levantamento de mensages de erro de um passo.

#### OpenMessageEditorForStep

Evento criado para acoplar a função SetStoryMessageRequest ao dispatcher SetMessages do MessageEditor, para tratar a edição de mensages de erro de um passo.

#### OnClicked (DialogueButton)

Interação com o botão da aba DialogueBuilder, chama a rotina de tratamento OnDialogueBuilderButtonClicked que realiza o carregamento da aba.

#### OnClicked (DialogueManagerInstance)

Interação com o botão DialogueManagerInstance do conteúdo do widget switcher exibido se não há um DialogueManager na cena. Chama a rotina OnCreateDialogueManagerClicked para gerenciar a adição de uma instância da classe na cena e, logo em seguida, chama OnDialogueBuilderButtonClicked para atualizar a aba do DialogueBuilder.


#### OnClicked (RefreshDialogueList)

Interação com o botão de recarregar da aba do DialogueBuilder, recarrega a lista de perguntas exibidas através da rotina UpdateDialogueList.

#### OnDialogueChanged

Método originário da interface DialogueListManagerInterface, chama a função OnDialogueListItemChanged que interage com o DialogueManager na cena e atualiza o seu conteúdo.

#### OnDialogueDeleted

Método originário da interface DialogueListManagerInterface, chama a função OnDialogueListItemDeleted que interage com o DialogueManager na cena e atualiza o seu conteúdo.

#### OnClicked (AddDialogueStep)

Interação com o botão AddDialogueStep da aba do DialogueBuilder, chama a rotina OnAddDialogueClicked para inserir uma nova pergunta na lista.

#### OnDialogueAnswerChanged

Método originário da interface AnswerListManagerInterface, chama a função OnAnswerItemChanged que interage com o DialogueManager na cena e atualiza o seu conteúdo.

#### OnDialogueAnswerDeleted

Método originário da interface AnswerListManagerInterface, chama a função OnAnswerItemDeleted que interage com o DialogueManager na cena e atualiza o seu conteúdo.

### Funções

#### OnAnchorButtonClicked

Carrega a aba do Anchors Manager.

Inicia fixando o widget switcher na aba 0, que contém a mensagem de AnchorsManager não encontrado na cena, e, em seguida, chama o método TryFindAnchorsManager, para verificar se há de fato uma instância do AnchorsManager e se há necessidade de trocar a aba do widget switcher. Também executa o TryFindStoryTeller e o TryFindDialogueManager para manter ambas as abas atualizadas.

#### OnStoryTellerButtonClicked

Carrega a aba do StoryTeller.

Inicia fixando o widget switcher na aba 0, que contém a mensagem de StoryTeller não encontrado na cena, e, em seguida, chama o método TryFindStoryTeller, para verificar se há de fato uma instância do StoryTeller e se há necessidade de trocar a aba do widget switcher. Também executa o TryFindAnchorsManager e o TryFindDialogueManager para manter ambas as abas atualizadas. 
Além disso, executa o TryCreateErrorManager que verifica se há um BP_Error_Widget_Manager, adicionando-o caso não exista. Isso, pois o Error Widget é necessário para a execução correta do StoryTeller.

#### OnDialogueBuilderButtonClicked

Carrega a aba do DialogueBuilder.

Inicia fixando o widget switcher na aba 0, que contém a mensagem de DialogueManager não encontrado na cena, e, em seguida, chama o método TryFindDialogueManager, para verificar se há de fato uma instância do DialogueManager e se há necessidade de trocar a aba do widget switcher. Também executa o TryFindAnchorsManager e o TryFindStoryTeller para manter ambas as abas atualizadas. 
Além disso, executa o TryCreateErrorManager que verifica se há um BP_Error_Widget_Manager, adicionando-o caso não exista. Isso, pois o Error Widget é necessário para a execução correta do DialogueManager.

#### TryFindAnchorsManager

Método que valida a existência do AnchorsManager na cena.

Busca um objeto da classe BP_AnchorsManager e, se for encontrado, troca o widget switch da aba AnchorsManager para exibir o conteúdo de edição de âncoras, chama os métodos UpdateAnchorsList e o TryFindAllAnchorsInScene para sincronizar o conetúdo da aba.

Se não for encontrado um AnchorsManager, reforça que o widget switcher fique na tela que informa isso e contém o botão de adicionar uma instância do AnchorsManager.

#### TryFindStoryTeller

Método que valida a existência do StoryTeller na cena.

Busca um objeto da classe BP_Bubble_StoryTellerBase e, se for encontrado, troca o widget switch da aba StoryTeller para exibir o conteúdo de edição de histórias, chama os métodos BuildStoryClassMap e o UpdateStoriesList para sincronizar o conetúdo da aba.

Se não for encontrado um StoryTeller, reforça que o widget switcher fique na tela que informa isso e contém o botão de adicionar uma instância do StoryTeller.

#### TryFindDialogueManager

Método que valida a existência do DialogueManager na cena.

Busca um objeto da classe BP_Dialogue_Widget_Manager e, se for encontrado, troca o widget switch da aba DialogueBuilder para exibir o conteúdo de edição de diálogo e chama o método UpdateDialogueList para sincronizar o conetúdo da aba.

Se não for encontrado um DialogueManager, reforça que o widget switcher fique na tela que informa isso e contém o botão de adicionar uma instância do DialogueManager.

#### TryFindAllAnchorsInScene

Método que popula o dropdown de âncoras na cena da aba AnchorsManager.

Inicia limpando os elementos da variável do tipo mapa AnchorNameToObjectReference e adiciona como primeiro opção uma mensagem de nenhuma âncora selecionada sem referência a um objeto. Em seguida, busca todos os objetos da classe BP_Anchor em cena e adiciona o nome de cada um e sua referência ao mapa.

Após isso, limpa todas os elementos do dropdown, varre as chaves da variável mapa e insere no dropdown. Finalmente, deixa como opção selecionada o primeiro elemento das chaves, a mensagem de nenhuma âncora selecionada.

#### TryCreateErrorManager

Método que valida e cria se necessário o ErrorWidgetManager.

Busca um objeto da classe BP_Error_Widget_Manager. Se não existir, instancia um na posição (0, 0, 0).

#### OnCreateAnchorManagerClicked

Instancia um AnchorsManager na posição (0, 0, 0) e salva a referência na variável AnchorsManager.

#### OnCreateStoryTellerClicked

Instancia um StoryTeller na posição (0, 0, 0) e salva a referência na variável StoryTeller.

#### OnCreateDialogueManager

Instancia um DialogueManager na posição (0, 0, 0) e salva a referência na variável DialogueManager. Define os parâmetros de posição para (845, 90, 100) e rotação (0, 0, 225), valores escolhidos de acordo com a posição desejada para o banner de diálogo dentro da simulação.

#### UpdateAnchorsList

Método que recarrega a lista de tags de âncoras da aba AnchorsManager.

Inicia limpando todos os itens da lista. Em seguida, chama o método GetAnchorsNamesEditorProperty que retorna a lista de tags do AnchorsManager em cena. Itera por cada um dos elementos do array e constrói um objeto do tipo AnchorTagListEntryObject. Esse objeto é adicionado ao widget de lista de tags de âncoras, acionando o seu método OnListItemObjectSet e populando a lista. Finalmente, chama o método UpdateAnchorSelectableDropdown para atualizar o conteúdo do dropdown de tags para uma âncora selecionada, para exibir possíveis alterações na lista de tags.

#### UpdateStoriesList

Método que recarrega a lista de histórias da aba StoryTeller.

Inicia limpando todos os itens da lista. Em seguida, chama o método GetStoryTellerStoriesEditorProperty que retorna a lista de BubbleStories do StoryTeller em cena. Itera por cada um dos elementos do array e constrói um objeto do tipo StoryListEntryObject. Esse objeto é adicionado ao widget de lista de histórias, acionando o seu método OnListItemObjectSet e populando a lista. 

#### UpdateDialogueList

Método que recarrega a lista de diálogos da aba DialogueBuilder.

Inicia limpando todos os itens da lista. Em seguida, chama o método GetDialogueManagerDialogueEditorProperty que retorna a lista de diálogos do DialogueManager em cena. Itera por cada um dos elementos do array e constrói um objeto do tipo DialogueListEntryObject. Esse objeto é adicionado ao widget de lista de diálogos, acionando o seu método OnListItemObjectSet e populando a lista. 

#### UpdateAnchorSelectableDropdown

Método que recarrega o dropdown de tags para uma âncora selecionada, da parte inferior da aba do AnchorsManager.

Inicia verificando se o dropdown está visível. Se estiver, chama a função GetAnchorsNamesEditorProperty que retorna as tags registradas no AnchorsManagers. Em seguida, limpa o dropdown e adiciona a opção de nenhuma tag selecionada. Após isso, varre o array de tags, adicionando cada uma ao dropdown.

Então, busca-se a âncora selecionada e a sua tag. Com isso, compara-se a tag da âncora com a lista do dropdown e se ela existir na lista, ela é selecionada na lista. Do contrário, deixa a primeira opção, com a mensagem de nenhuma tag selecionada. 

#### OnAddAnchorClicked

Método que adiciona uma tag padrão na lista de tags.

Chama o método GetAnchorsNamesEditorProperty e passa para uma variável local. Adiciona à lista local uma tag com valor padrão e insere os conteúdos do array local no AnchorsManager através do método SetAnchorsNamesEditorProperty. Finalmente, chama o método UpdateAnchorsList, para atualizar as tags exibidas, sincronizando-as com o valor no AnchorsManager.

#### OnAddStoryClicked

Método que adiciona uma história vazia na lista de histórias.

Chama o método GetStoryTellerStoriesEditorProperty e passa para uma variável local. Adiciona à lista local um struct vazio e insere os conteúdos do array local no StoryTeller através do método SetStoryTellerStoriesEditorProperty. Finalmente, chama o método UpdateStoriesList, para atualizar as histórias exibidas, sincronizando-as com o valor no StoryTeller.

#### OnAddDialogueClicked

Método que adiciona uma história vazia na lista de histórias.

Chama o método GetDialogueManagerDialoguesEditorProperty e passa para uma variável local. Adiciona à lista local um struct vazio e insere os conteúdos do array local no DialogueManager através do método SetDialogueManagerDialogueEditorProperty. Finalmente, chama o método UpdateDialoguesList, para atualizar as histórias exibidas, sincronizando-as com o valor no DialogueManager.

#### OnApplyAnchorTagToAnchor

Método que altera o valor da tag de uma âncora.

Chama o método GetAnchorsNamesEditorProperty para receber a lista de tags de âncora registradas. Verifica se o valor selecionado no dropdown de tags para uma âncora selecionada está dentro do array de tags. Se estiver, utiliza o nome da âncora selecionada para recuperar a referência ao objeto com o mapa AnchorNameToObjectReference e chama o método SetAnchorTagValue da âncora, passando a tag validada.

#### OnAnchorTagListItemChanged

Método que atualiza o conteúdo da lista de tags do AnchorsManager.

Chama o método GetAnchorsNamesEditorProperty para receber a lista de tags de âncoras registradas no AnchorsManager e salva em uma array local. Utiliza a função SetArrayElem para realizar alterações no array local e salva as alterações no AnchorsManager através do método SetAnchorsNamesEditorProperty. Finalmente, chama o método UpdateAnchorsList para sincronizar a lista de tags de âncoras exibidas com o AnchorsManager.

#### OnAnchorTagListItemDeleted

Método que remove um elemento da lista de tags do AnchorsManager.

Chama o método GetAnchorsNamesEditorProperty para receber a lista de tags de âncoras registradas no AnchorsManager e salva em uma array local. Utiliza a função RemoveIndex para deletar a tag do array local e salva as alterações no AnchorsManager através do método SetAnchorsNamesEditorProperty. Finalmente, chama o método UpdateAnchorsList para sincronizar a lista de tags de âncoras exibidas com o AnchorsManager.

#### OnStoryListItemChanged

*** 

#### OnStoryListItemDeleted

Método que remove um elemento da lista de histórias do StoryTeller.

Chama o método GetStoryTellerStoriesEditorProperty para receber a lista de histórias registradas no StoryTeller e salva em uma array local. Utiliza a função RemoveIndex para deletar a história do array local e salva as alterações no StoryTeller através do método SetStoryTellerStoriesEditorProperty. Finalmente, chama o método UpdateStoryList para sincronizar a lista de histórias exibidas com o StoryTeller.

#### OnDialogueListItemChanged

Método que atualiza o conteúdo da lista de diálogos do DialogueManager.

Chama o método GetDialogueManagerDialoguesEditorProperty para receber a lista de tags de âncoras registradas no DialogueManager e salva em uma array local. Utiliza a função SetArrayElem para realizar alterações no array local e salva as alterações no DialogueManager através do método SetDialogueManagerDialoguesEditorProperty. Finalmente, chama o método UpdateDialoguesList para sincronizar a lista de diálogos exibidos com o DialogueManager.

#### OnDialogueListItemDeleted

Método que remove um elemento da lista de diálogos do StoryTeller.

Chama o método GetDialogueManagerDialoguesEditorProperty para receber a lista de diálogos registradas no DialogueManager e salva em uma array local. Utiliza a função RemoveIndex para deletar o diálogo do array local e salva as alterações no DialogueManager através do método SetDialogueManagerDialoguesEditorProperty. Finalmente, chama o método UpdateStoryList para sincronizar a lista de diálogos exibidos com o DialogueManager.

#### OnAnswerItemChanged

***

#### OnAnswerItemDeleted

***

#### BuildStoryClassMap

Método que constrói o mapa de strings para referências de classes que herdam de BP_Bubble_StoryBase.

Chama o método GetAllClassesDerived para obter um array com as classes que herdam do StoryBase e incluem o StoryBase, limpa o mapa StoryClassNameToClassReference e itera o array de classes, inserindo a cada item o nome como chave e a classe como conteúdo dentro do mapa.

#### BuildStepClassMap

Método que constrói o mapa de strings para referências de classes que herdam de BP_StepBase.

Chama o método GetAllClassesDerived para obter um array com as classes que herdam do StepBase e incluem o StepBase, limpa o mapa StepClassNameToClassReference e itera o array de classes, inserindo a cada item o nome como chave e a classe como conteúdo dentro do mapa.

#### GetAnchorsNamesEditorProperty

Método que retorna a lista de tags do AnchorsManager.

Utilizando o método GetEditorProperty e o nome da propriedade com as tags, retorna um array com as tags.

#### SetAnchorsNamesEditorProperty

Método que sobreescreve a lista de tags do AnchorsManager.

Utilizando o método SetEditorProperty, o nome da propriedade com as tags e o novo array com as tags alteradas, atualiza o valor da propriedade do AnchorsManager.

#### GetStoryTellerStoriesEditorProperty

Método que retorna a lista de histórias do StoryTeller.

Utilizando o método GetEditorProperty e o nome da propriedade com as histórias, retorna um array com as histórias.

#### SetStoryTellerStoriesEditorProperty

Método que sobreescreve a lista de histórias do StoryTeller.

Utilizando o método SetEditorProperty, o nome da propriedade com as histórias e o novo array com as histórias alteradas, atualiza o valor da propriedade do StoryTeller.

#### GetDialogueManagerDialogueEditorProperty

Método que retorna a lista de diálogos do DialogueManager.

Utilizando o método GetEditorProperty e o nome da propriedade com os diálogos, retorna um array com as diálogos.

#### SetDialogueManagerDialogueEditorProperty

Método que sobreescreve a lista de diálogos do DialogueManager.

Utilizando o método SetEditorProperty, o nome da propriedade com os diálogos e o novo array com os diálogos alterados, atualiza o valor da propriedade do DialogueManager.

#### TreatGetStoryMessageRequest

#### TreatSetStoryMessageRequest

#### TreatSetStepMessageRequest

#### TreatGetStepMessageRequest

#### GetStoryMessagesByIndex

#### SetStoryMessagesByIndex

#### GetStepMessagesByIndex

#### SetStepMessagesByIndex

#### GetAllClassesDerived

#### NewAnchorSelected

#### SpawnMessageEditor