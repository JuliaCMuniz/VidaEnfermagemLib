# Widget DialogListEntryWidget

Widget de um elemento da lista de questões do diálogo, contém três inputs numéricos para definir os parâmetros RGBA da cor da resposta (exibida ao lado dos inputs), um input de texto com múltiplas linhas para a questão, um botão de expansão para exibir as respostas da questão e um botão de remoção do elemento.

## Graph

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato AnswerListEntryObject dentro do editor widget StoryBuilder. 

Define o conteúdo do botão de expansão como "+" e o index do AnswerContent Widget Switcher para 0, exibindo o estado colapsado das respostas da questão. Configura a variável DialogueListEntryObj do widget para referenciar o objeto de entrada e chama a rotina UpdateValues que preenche o item com os valores do objeto.

#### OnTextCommited (RValue, GValue, BValue, AValue)

Após qualquer um dos campos de texto numérico perderem o foco, chama a rotina UpdateColor para atualizar o display de cor, chama a rotina OnItemChanged responsável por acionar os métodos da interface DialogueListManagerInterface para atualizar o conteúdo salvo da reposta e, por fim, chama a rotina UpdateValues, para sincronizar o item com o conteúdo salvo.

#### OnClicked (RemoveDialogueButton)

Após o botão de remoção ser clicado, chama a rotina OnItemDeleted, que aciona a interface DialogueListManagerInterface para remover a questão. 

#### OnTextCommited (Multi-Line Text Box) (QuestionContent)

Após a perda de foco no campo da questão, chama a rotina UpdateQuestionContent que gerencia a alteração de valor do campo de texto, chama a rotina OnItemChanged que aciona a interface DialogueListManagerInterface para salvar as alterações da questão e chama a função UpdateValues para sincronizar o item com a versão salva.

#### OnClicked (ExpandDialogueButton)

Após interagir com o botão de expandir ou colapsar o conteúdo das respostas da questão, aciona a rotina ToggleAnswerContent que gerencia as mudanças de estado da visibilidade das respostas da questão.

#### OnClicked (AddAnswerButton)

Após interagir com o botão de adicionar uma resposta à questão, aciona a interface DialogueListManagerInterface para chamar o método AddAnswerToDialogue e, em seguida, o método PopulateAnswers para sincronizar a lista de respostas com a adição.

#### OnClicked (RefreshAnswerButton)

Após interagir com o botão de atualizar a lista de respostas, aciona a interface DialogueListManagerInterface para chamar o método GetDialogueAnswers e, em seguida, o método PopulateAnswers para sincronizar a lista de respostas com a nova busca de respostas.

### Funções

#### UpdateValues

A partir da variável DialogueListEntryObj, configura os valores dos campos de texto, da checkbox, chama a função SetRGBA para definir os valores dos campos numéricos e chama a função UpdateColor para atualizar o display de cor do item. 

#### SetRGBA

Converte a propriedade de cor do objeto referenciado na variável DialogueListEntryObj para string e altera os campos de texto numérico do item.

#### UpdateColor

Formata as strings dos campos numéricos para utilizarem '.' caso ',' seja inserido no input e converte seus conteúdos para um float. Assim, utiliza os quatro floats para compor um objeto de cor, atualizando a variável CurrentColor e construindo um novo objeto do tipo DialogueListEntryObject para substituir o valor da variável DialogueListEntryObj.

O display de cor corresponde a uma Border dentro de uma SizeBox em que o BrushColor tem um bind com a variável CurrentColor, permitindo que ele seja atualizado de forma dinâmica.  

#### OnItemChanged

Encontra o widget com a interface DialogueListManagerInterface e chama o método OnDialogueChanged, utilizando o valor atual da variável DialogueListEntryObj.

#### OnItemDeleted

Aciona a interface DialogueListManagerInterface e chama o método OnDialogueDeleted, utilizando o index fornecido pelo valor atual da variável DialogueListEntryObj.

#### UpdateQuestionContent

Cria um novo objeto da classe DialogueListEntryObject contendo o valor atualizado do texto da questão e atualiza a referência da variável DialogueListEntryObj para esse objeto.

#### ToggleAnswerContent

A partir do valor atual do Widget Switcher, troca para ou 0 ou 1, habilitando um container vazio ou a lista de respostas respectivamente. Além disso, atualiza o texto do botão de expansão para exibir "+" se as respostas estiverem colapsadas e "-" no caso contrário. Finalmente, após habilitar a lista de respostas com o Widget Switcher, chama a função GetDialogueAnswers para buscar as respostas com a interface DialogueListManagerInterface e chama a função PopulateAnswers para atualizar a lista de respostas exibidas.

#### GetDialogueAnswers

Aciona a interface DialogueListManagerInterface e chama o método GetDialogueAnswers para obter as respostas associadas à questão do item. 

#### PopulateAnswers

Rotina para atualizar a lista de respostas exibidas. 

Inicia com a limpeza da lista, seguida por um loop varrendo as respostas recebidas e, a cada iteração, cria um objeto da classe AnswerListEntryObject contendo os valores obtidos com a chamada da interface e insere no widget de lista de respostas, acionando o seu método OnListObjectSet e populando a lista.

