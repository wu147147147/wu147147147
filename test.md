# 🎓 NTHU Course Preference Agent - System Integration & Data Specification (v3.5)

This document defines the data interfacing standards, data cleaning pipelines, and state machine control specifications between the **NTHU Course Preference Agent** (Frontend NLP Module) and the **Google OR-Tools CP-SAT Solver** (Backend Optimization Engine). It serves as the single source of truth for the development team during GitHub code reviews and commit alignments.

---

## 🏗️ System Architecture & Workflow

The Preference Agent acts as a **"Translator"** bridging ambiguous human intentions with strict mathematical machine constraints. The core runtime pipeline executes as follows:
1. **Data Synchronization**: The system fetches the official NTHU Open Data JSON (encoded in UTF-16) daily, cleanses the strings, and instantiates a local memory database cache.
2. **Semantic Parsing**: The LLM (Gemini 2.5 Flash) interprets user chat inputs and maps conversational context against the course attributes in the cached background knowledge.
3. **Preference Quantification**: The LLM generates a structured JSON output compliant with a predefined Pydantic schema, computing a dynamic `bias` matrix.
4. **Decision Persistence**: Upon reaching the `complete` state, the agent persists the final `bias` matrix into `NTHU_Course_Bias_Output.json`, which acts as the direct coefficient input for the backend CP-SAT optimizer.

---

## 📊 1. Data Source: NTHU Official Open Data Registry

The agent interfaces with the official university course registry node:
`https://www.ccxp.nthu.edu.tw/ccxp/INQUIRE/JH/OPENDATA/open_course_data.json`

### 🔍 Raw Fields Specification & Data Cleaning Rules
To prevent control character bugs and anomalies from corrupting the LLM's prompt window, the agent must enforce the following parsing rules during initial loading:

| Official Field Name (Traditional Chinese) | Max Length | Team Cleaning & Parsing Standards |
| :--- | :---: | :--- |
| **科號** (Course No.) | 15 | **Primary Key**. The in-memory cache must double-index both the `Standard Spaced` variant (e.g., `11420CS 460100`) and the `Flat Compressed` variant (e.g., `11420CS460100`) to remain robust against LLM output mutations. |
| **課程中文名稱** (Chinese Title) | 40 | Base semantic attribute. |
| **課程英文名稱** (English Title) | 100 | Auxiliary semantic attribute. |
| **學分數** (Credits) | 5 | Parsed into a clean integer/string token used to evaluate baseline efficiency. |
| **停開註記** (Status Note) | 4 | If the value matches **"停開"** (Cancelled), the record **MUST be dropped immediately**. It must not enter the cache or context injection. |
| **授課教師** (Instructor) | 1000 | Contains multiple instructors (`\n` delimited) and bilingual names (`\t` delimited).<br>👉 **Rule**: Split by `\n`, then extract the first token before `\t` to **keep Chinese names only**. Join via a comma `,` (e.g., `丁川康`). |
| **教室與上課時間** (Schedule/Room) | 150 | Maps rooms to time strings via `\t`, with multiple pairings split by `\n`.<br>👉 **Rule**: Split tokens and **strip out room characters** containing "館, 樓, 室, 房". **Pass only the pure time tokens** (e.g., `T3T4R4`) to the LLM to minimize context token overhead. |
| **授課語言** (Language) | 2 | "中" (Chinese) or "英" (English). If "英", prepend an `[EMI]` tag to the context title for user filtration capability. |
| **通識類別** (GE Category) | 100 | Used as the heuristic boundary to selectively retain General Education courses in the LLM's background knowledge pool. |

> ⚠️ **CRITICAL ENCODING NOTE**: The NTHU registry streams raw data using **`UTF-16`** encoding. HTTP connection scripts must explicitly declare `response.encoding = 'utf-16'` to prevent payload corruption. Local cache storage must be normalized to standard `UTF-8`.

---

## 🤖 2. Agent Structured Output Schema (Pydantic V2)

To enforce strict formatting bounds on the LLM outputs, Gemini's **Structured Outputs (`response_schema`)** is fully enabled.

### 📋 JSON Schema Structure (EnhancedResponseSchema)
```json
{
  "status": "need_clarification | need_confirmation | complete",
  "message_to_user": "string (The narrative dialog shown to the student)",
  "scheduling_data": [
    {
      "科號": "string (Course_ID matching the NTHU registry key precisely)",
      "bias": "float (The quantifiable preference weight mapped to the objective function)"
    }
  ]
}
📐 Bias Quantification StandardsThe bias attribute directly impacts the optimization landscape of the backend CP-SAT solver objective function. Prompt engineers must direct the LLM to quantize user sentiments according to these boundaries:Strong Affinities (Compulsory/Highly Favored): If a user states a course is a critical requirement or a top preference, the bias spikes significantly (e.g., +50.0 to +100.0), giving it a mathematical priority in the solver execution.Soft Constraints Penalization (Avoidance): If a user expresses dislike for a professor, blacklists an early 8:00 AM slot, or has already cleared the course, the bias plummets (e.g., -50.0 to -100.0), telling the solver to naturally drop the course unless absolutely necessary to clear hard credit boundaries.Physical Education (PE) Safe-guard: PE classes typically carry low academic weights (1 credit over 2 hours, making $\frac{\text{credits}}{\text{slots}} = 0.5$). To prevent the optimizer from naturally starving out PE options, the LLM must inject a compensatory bias (e.g., $\text{bias} \ge +30.0$) whenever a user requests an athletic course placement.⚙️ Dialog State Machine ControlsThe status field strictly manages session progression and JSON compilation dumps across three states:need_clarification: Activated if user intent is too vague (e.g., "Help me make a schedule"). The LLM outputs zero scheduling records and utilizes message_to_user to gather specific subject or structural preferences.need_confirmation: Triggered when the LLM successfully maps keywords to valid NTHU course numbers and populates a tentative scheduling_data matrix. The user interface renders this matrix, and message_to_user prompts the user to verify the bias allocations.complete: Handshaked only when the user explicitly yields an affirmative confirmation phrase (e.g., "Yes", "That looks right", "Perfect"). The terminal script intercepts this tag, commits the data to file storage, and resets the pipeline.💾 3. Optimizer Interfacing: NTHU_Course_Bias_Output.jsonUpon reaching the complete status, the final matrix is generated as a structured file in the workspace directory. This acts as the vector arrays that map directly to the backend Integer Linear Programming (ILP) model.📐 Mathematical Paradigm (CP-SAT Formulation)The backend OR-Tools solver models the scheduling problem as an absolute constraint satisfaction network. The objective function is formulated as:$$ \max f(S) = \sum_{i=0}^{N-1} \text{CUtil}(c_i) \cdot s_i $$Where the cumulative utility coefficient $\text{CUtil}(c_i)$ for any course variable $s_i$ is computed as:$$ \text{CUtil}(c_i) = \frac{\text{credits}_i}{\text{slots}_i} + \text{bias}_i $$$s_i \in \{0, 1\}$: The binary decision variable managed by cp_model.BoolVar().$\text{bias}_i$: The weight parsed, calculated, and exported directly by this NLP Agent module.📄 Production Export Payload ExampleJSON[
    {
        "科號": "11420CS 460100",
        "bias": 80.0
    },
    {
        "科號": "11420EECS101001",
        "bias": -50.0
    }
]

⚙️ 4. Local Deployment & DependenciesThe repository demands a local .env setup at the project root node (which is explicitly appended to your .gitignore rules):程式碼片段GEMINI_API_KEY=your_secured_gemini_api_key_here
📦 Mandatory Python Ecosystem PackagesAll team members must normalize their local working environments using the following dependency stack:Bashpip install google-genai pydantic requests python-dotenv --user
