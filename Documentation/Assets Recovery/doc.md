# 🔧 Recuperação de Assets

## 👤 Paciente

### 📋 Contexto
As relações presentes no asset do paciente original foram perdidas durante a migração de versão do projeto. Animações, texturas e malhas esqueléticas do paciente foram comprometidas. Adicionalmente, diversos nós Blueprints do ator Paciente apresentavam dependências do Leap Motion, tornando urgente a recuperação das relações do asset e a eliminação dessas dependências através de nova lógica.

### 🔄 Tentativas de Recuperação

Durante a migração de versão do projeto, o asset do paciente perdeu relações críticas (animações, texturas, malhas esqueléticas) e apresentava dependências problemáticas do Leap Motion nos nós Blueprint. Três tentativas de correção foram realizadas:

1. **🔍 Comparação Manual**: Comparação entre versões e substituição de nós dependentes do Leap Motion → resultou em bug onde o paciente era arrastado para trás da cadeira durante a transição Sentando→Sentado

2. **📥 Importação Nativa**: Importação de arquivos .fbx das animações originais via Unreal → resolveu o problema inicial mas manteve o arrasto durante a transição

3. **🎨 Recriação via Mixamo**: Recriação do modelo Remy e animações (Sitting/Sitted) com root motion aplicado no Blender, assumindo erro no osso do quadril original → sem sucesso

### ✅ Solução Final

A solução foi reimportar as animações originais aplicando uma **translação específica no eixo X** durante a importação. Isso corrigiu a discrepância de pontos de origem no plano cartesiano entre as animações Sentando e Sentado que causava o erro de transição.