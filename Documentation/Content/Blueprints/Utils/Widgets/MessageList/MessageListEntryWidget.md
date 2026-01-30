# Widget MessageListEntryWidget

Widget de um elemento da lista de mensagens de erro de uma história ou passo, contém um campo de texto editável com múltiplas linhas e um botão de remoção do elemento. Além disso, contém um título não editável associando cada mensagem a um índice. O índice é composto pelo index da mensagem acrescido de 1.

## Graph

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato MessageListEntryObject dentro do editor widget StoryBuilder. 
    
Configura a variável MessageListEntry do widget para referenciar o objeto de entrada e chama o método UpdateValues para atualizar o campo de texto do widget.

#### OnTextCommited (Editable Text) (Content)

Após o campo de texto editável perder o foco, atualiza o objeto referenciado pela variável MessageListEntry para conter o novo valor de texto, utiliza a referência do widget MessageEditor presente na variável para chamar o método OnMessageChanged para salvar as alterações da mensagem de erro.

#### OnClicked (DeleteButton)

Após o botão de remoção ser clicado, utiliza a referência do MessageEditor para chamar o método OnMessageDeleted, passando o valor de index recebido do objeto de entrada. 

### Funções

#### UpdateValues

Atualiza o conteúdo da mensagem de erro e do título associado a partir do conteúdo da variável MessageListEntry.

