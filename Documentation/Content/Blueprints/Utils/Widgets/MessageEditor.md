# Widget MessageEditor

Widget contendo o ambiente de edição das mensagens de erro. Apresenta um botão para adicionar mensagens, outro para atualizar a lista de mensagens e a lista de widgets do tipo MessageListEntryWidget. Este widget implementa a interface MessageListManagerInterface, que dialoga com os widgets do tipo MessageListEntryWidget.

## Graph

### Variáveis

#### Messages

Array de strings que é atualizado pelo editor widget StoryBuilder para manter a lista de mensagens de erro do item (história ou passo) que chamou a edição.

#### SuperIndex

Caso a mensagem de erro pertença a um passo, SuperIndex contém o valor de index da história à qual ele pertence.

#### Index

Índice da história ou passo cujas mensagens de erro estão em edição.

### EventGraph

#### OnClicked (Refresh), BeginRoutines

Chama o dispatcher GetMessages, iniciando as rotinas de atualização da variável Messages do StoryBuilder, e chama a função PopulateList, para sincronizar a lista exibida com a nova lista buscada.

#### OnClicked (AddMessage)

Adiciona um item vazio no array Messages e chama o dispatcher SetMessages, que inicia as rotinas do StoryBuilder para salvar as mensagens. No fim, chama PopulateList para atualizar a exibição de mensagens para conter o novo item do array.

#### OnMessageChanged

Evento chamado pelos MessageListEntryWidgets. Passa o conteúdo da mensagem alterada para a variável Messages, chama SetMessage, iniciando as rotinas do StoryBuilder para salvar as mensagens, e chama PopulateList, para sincronizar a lista com o que foi salvo.

#### OnMessageDeleted

Evento chamado pelos MessageListEntryWidgets. Remove o elemento correspondente do array de mensagens. Chama SetMessage para iniciar as rotinas de permanências das mensagens no StoryBuilder e chama PopulateList para sincronizar a lista com o que foi salvo.

### Funções

#### PopulateList

Rotina para atualizar a lista de mensagens exibidas. 

Inicia com a limpeza da lista, seguida por um loop varrendo a variável Messages e, a cada iteração, cria um objeto da classe MessageListEntryObject contendo e insere no widget de lista de mensagens, acionando o seu método OnListObjectSet e populando a lista.

Nota: foi escolhido o método de popular a variável dos objetos de lista após sua criação ao invés de instanciar o objeto inicializado com o conteúdo da variável, pois percebeu-se que havia um problema de cache que afetava o funcionamento correto de rotinas de atualização de valor dos itens da lista.

### Dispatchers

#### GetMessages

Dispatcher que se comunica com o widget StoryBuilder. Caso as mensagens de erro pertençam a uma história, é acoplado ao evento que busca as mensagens da história utilizando o index. Do contrário, é acoplado ao evento que busca as mensagens de um passo utilizado o index e o Superindex.

#### SetMessages

Dispatcher que se comunica com o widget StoryBuilder. Caso as mensagens de erro pertençam a uma história, é acoplado ao evento que altera as mensagens da história utilizando o index. Do contrário, é acoplado ao evento que altera as mensagens de um passo utilizado o index e o Superindex.