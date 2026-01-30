# Widget AnswerListEntryWidget

Widget de um elemento da lista de respostas de diálogo, contém três inputs numéricos para definir os parâmetros RGBA da cor da resposta (exibida ao lado dos inputs), dois inputs de texto com múltiplas linhas para a resposta e sua justificativa, uma checkbox para indicar se a resposta é correta ou não e um botão de remoção do elemento.

## Graph

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato AnswerListEntryObject dentro do editor widget StoryBuilder. 

Configura a variável AnswerListEntryObj do widget para referenciar o objeto de entrada e chama a rotina UpdateValues que preenche o item com os valores do objeto.

#### OnClicked (RemoveAnswerButton)

Após o botão de remoção ser clicado, encontra o widget com a interface AnswerListManagerInterface e chama o método OnDialogueAnswerDeleted, passando os valores de index recebidos do objeto de entrada. 

#### OnTextCommited (RValue, GValue, BValue, AValue)

Após qualquer um dos campos de texto numérico perderem o foco, chama a rotina UpdateColor para atualizar o display de cor, chama a rotina OnItemChanged responsável por acionar os métodos da interface AnswerListManagerInterface para atualizar o conteúdo salvo da reposta e, por fim, chama a rotina UpdateValues, para sincronizar o item com o conteúdo salvo.

#### OnCheckStateChanged (Check Box), OnTextCommited (Multi-Line Text Box) (AnswerContent), OnTextCommited (Multi-Line Text Box) (AnswerExplanation)

Após alterações ou perda de foco nos demais campos de entrada, cria um novo objeto da classe AnswerListEntryObject contendo o valor alterado, atualiza a variável AnswerListEntryObj para referenciar esse novo objeto, chama o método OnItemChanged, para salvar o conteúdo da resposta, e chama a rotina UpdateValues, para sincronizar o item com o conteúdo salvo.

### Funções

#### UpdateColor

Formata as strings dos campos numéricos para utilizarem '.' caso ',' seja inserido no input e converte seus conteúdos para um float. Assim, utiliza os quatro floats para compor um objeto de cor, atualizando a variável CurrentColor e construindo um novo objeto do tipo AnswerListEntryObject para substituir o valor da variável AnswerListEntryObj.

O display de cor corresponde a uma Border dentro de uma SizeBox em que o BrushColor tem um bind com a variável CurrentColor, permitindo que ele seja atualizado de forma dinâmica.  

#### UpdateValues

A partir da variável AnswerListEntryObj, configura os valores dos campos de texto, da checkbox, chama a função SetRGBA para definir os valores dos campos numéricos e chama a função UpdateColor para atualizar o display de cor do item. 

#### SetRGBA

Converte a propriedade de cor do objeto referenciado na variável AnswerListEntryObj para string e altera os campos de texto numérico do item.

#### OnItemChanged

Encontra o widget com a interface AnswerListManagerInterface e chama o método OnDialogueAnswerChanged, utilizando o valor atual da variável AnswerListEntryObj.

