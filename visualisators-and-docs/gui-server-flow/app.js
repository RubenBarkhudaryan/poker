const flows = {
  startup: {
    title: "Startup and load",
    blurb: "Local mode launches TexasSolverServer, then the GUI loads the hand-ranking files.",
    steps: [
      ui(1, "Read settings", "MainWindow::connectBackend reads backend_mode, host and port. Local mode always uses 127.0.0.1.", "mainwindow.cpp", null),
      ui(2, "Start the bundled server", "ServerProcess starts TexasSolverServer.exe from the GUI folder, with the port as its argument and no console window.", "server_process.cpp", null),
      server(3, "Listen", "main() binds QTcpServer, installs the qDebug handler, and enters the Qt event loop. Nothing is loaded yet.", "src/server/main.cpp", null),
      ui(4, "Probe the port", "A background thread calls waitUntilReachable. Each try opens a new TCP connection and sends status.", "remote_backend.cpp", req("status")),
      server(5, "Answer status", "status and stop skip the busy check. status copies the event queue and then clears it.", "src/server/main.cpp", null),
      reply(6, "Server is up", "ok is true. busy is false and loaded is false, so the GUI knows it can send load.", null, statusReply(false, "", false)),
      ui(7, "Send load", "load() drops the cached ranks/board/stack, then startJobAndWait sends the command and waits.", "remote_backend.cpp", req("load")),
      server(8, "Queue the load job", "loaded_ is still false, so startJob sets busy and runs LocalBackend::load on a worker thread. The socket reply is sent immediately.", "src/server/main.cpp", null),
      reply(9, "Job accepted", "result is null. The files are not loaded yet. The GUI must poll.", null, ok(null)),
      server(10, "Load the Hold'em solver", "The worker builds the single Hold'em PokerSolver from the compairer resources and pushes progress lines.", "local_backend.cpp", null),
      ui(11, "Poll every 300 ms", "A new connection asks for status. Progress events are written into the log window.", "remote_backend.cpp", req("status")),
      reply(12, "Load finished", "busy is false, loaded is true, error is empty. load() returns to the GUI.", null, statusReply(false, "", true, [
        { iteration: 0, exploitability: 0, message: "Loading finished. Good to go." }
      ]))
    ]
  },
  build: {
    title: "Build tree",
    blurb: "The Build button packs every street setting into one SolveRequest.",
    steps: [
      ui(1, "Read the form", "fillRequestFromUi copies ranges, board, pot, stack, threads and all six street bet/raise/donk settings into SolveRequest.", "mainwindow.cpp", null),
      ui(2, "Leave the UI thread", "runInBackground refuses a second job, then runs buildTree on a QThread.", "mainwindow.cpp", null),
      ui(3, "Send the full request", "buildTree invalidates the info cache and sends the whole SolveRequest as args. One JSON line, ended by a newline.", "remote_backend.cpp", req("build", sampleRequest())),
      server(4, "Accept the line", "readyRead waits for a full line, parses JSON, and calls dispatch(\"build\", args).", "src/server/main.cpp", null),
      server(5, "Check guards", "If load or build is already running, the server throws \"busy\". Other commands also require loaded_ to be true.", "src/server/main.cpp", null),
      server(6, "Start the worker", "args become a SolveRequest. buildTree runs on the worker: bet sizes, raise limit, stack and board round.", "local_backend.cpp", null),
      reply(7, "Accepted at once", "The TCP reply does not wait for the tree. result is null and the socket closes.", null, ok(null)),
      ui(8, "Poll until quiet", "status is called on a fresh socket every 300 ms. Events land in the log. A non-empty error is thrown.", "remote_backend.cpp", req("status")),
      server(9, "Tree is readable again", "When the worker finishes, busy becomes false. getTreeRoot can now register node id \"0\".", "src/server/main.cpp", null),
      reply(10, "Build done", "The GUI returns from buildTree. The next Show Result click can ask for the root.", null, statusReply(false, "", true, [
        { iteration: 0, exploitability: 0, message: "build tree finished" }
      ]))
    ]
  },
  solve: {
    title: "Solve",
    blurb: "Solve is a long job. The explorer may still read strategy while it runs.",
    steps: [
      ui(1, "Take solver knobs", "The Solve button copies iteration, accuracy, print interval and thread count onto the last request.", "mainwindow.cpp", null),
      ui(2, "Send solve", "The full SolveRequest goes out, including ranges, board and tree settings already stored from Build.", "remote_backend.cpp", req("solve", sampleRequest())),
      server(3, "Reject only if the tree is changing", "A running load or build blocks every command except status and stop. A running solve does not block strategy reads.", "src/server/main.cpp", null),
      server(4, "Train on the worker", "LocalBackend::solve calls PokerSolver::train. qDebug lines are captured by the message handler into the event queue.", "local_backend.cpp", null),
      reply(5, "Started", "The GUI gets null immediately and begins polling. CFR is still running.", null, ok(null)),
      ui(6, "Poll and paint the log", "Each status reply's events are passed to the progress callback, which appends them to the log widget.", "remote_backend.cpp", req("status")),
      reply(7, "Still running", "busy stays true. events holds the iteration lines, then the server clears that queue.", null, statusReply(true, "solve", true, [
        { iteration: 0, exploitability: 0, message: "Iteration 1 exploitability 12.4" }
      ])),
      server(8, "Explorer can read", "A second socket may send strategy, root or children during solve. estimate, build and save wait until busy is false.", "src/server/main.cpp", null),
      reply(9, "Finished", "busy is false and error is empty. solve() returns. The info cache was cleared, so the next ranks/board read hits the network once.", null, statusReply(false, "solve", true))
    ]
  },
  stop: {
    title: "Stop",
    blurb: "Stop uses its own connection, so it is not stuck behind the solve poll.",
    steps: [
      ui(1, "Stop button", "on_stopSolvingButton_clicked calls backend_->stop() on the UI thread, inside a try/catch.", "mainwindow.cpp", null),
      ui(2, "Separate socket", "stop() is a single call(). It does not wait for the solve poll loop, which is blocked on its own socket.", "remote_backend.cpp", req("stop")),
      server(3, "Ask the solver to stop", "dispatch handles stop before the busy check. It calls LocalBackend::stop only when the current job is solve.", "src/server/main.cpp", null),
      server(4, "Solver notices", "PokerSolver::stop sets the flag the CFR loop checks. The worker then ends and clears busy.", "local_backend.cpp", null),
      reply(5, "Stop acknowledged", "result is null. The poll loop's next status sees busy false and solve() returns.", null, ok(null))
    ]
  },
  strategy: {
    title: "Show a strategy",
    blurb: "Clicking a tree node is one round trip. Ranks and stack stay cached.",
    steps: [
      ui(1, "Open the explorer", "Show Result calls hasGameTree(). That is its own short command, has_tree.", "mainwindow.cpp", req("has_tree")),
      reply(2, "Tree exists", "The server returns loaded_ && hasGameTree(). The explorer is created with the RemoteBackend pointer.", null, ok(true)),
      ui(3, "Click a node", "StrategyExplorer asks getStrategyAt(node id, turn card, river card). Turn and river are -1 when unused.", "strategyexplorer.cpp", req("strategy", { id: "0/1", turn: -1, river: -1 })),
      server(4, "Build the grid", "LocalBackend finds the node, reads strategy and EVs, and conditions both players' ranges along the path.", "local_backend.cpp", null),
      server(5, "Serialize every field", "The JSON includes actions, strategy, evs, both ranges, both combo lists, ranks and card names.", "protocol.cpp", null),
      reply(6, "One line back", "The GUI parses StrategyGridDto. TableStrategyModel fills the grid, EV view and range view from that object.", null, ok({
        current_player: 0,
        actions: [{ label: "CHECK", amount: 0 }, { label: "BET 50", amount: 50 }],
        strategy: "[[[0.62, 0.38], ...]]",
        evs: "[[[1.2, 0.4], ...]]",
        p1_range: "[[0.18, ...]]",
        p2_range: "[[0.07, ...]]"
      }))
    ]
  },
  tree: {
    title: "Expand the tree",
    blurb: "Each expanded node is one children request. The GUI does not walk GameTreeNode pointers anymore.",
    steps: [
      ui(1, "Ask for the root", "TreeModel::setupModelData calls getTreeRoot when the explorer opens.", "treemodel.cpp", req("root")),
      server(2, "Format node 0", "LocalBackend::toDto fills id, label, type, round, player, pot and has_children, and remembers the pointer under that id.", "local_backend.cpp", null),
      reply(3, "Root DTO", "The view inserts one visible root row. Children are not included in this reply.", null, ok({
        id: "0", label: "FLOP begin", node_type: "action", round: "flop", player: 0, pot: 50, has_children: true
      })),
      ui(4, "Expand a row", "reGenerateTreeItem sends the node's id. Child ids are the parent id plus /0, /1, ...", "treemodel.cpp", req("children", { id: "0" })),
      server(5, "List direct children only", "Action nodes emit one DTO per action. A chance node emits a single child with id ending in /0.", "local_backend.cpp", null),
      reply(6, "Child rows", "The model inserts those DTOs. Expanding one of them sends another children command.", null, ok([
        { id: "0/0", label: "OOP CHECK", node_type: "action", round: "flop", player: 1, pot: 50, has_children: true },
        { id: "0/1", label: "OOP BET 50", node_type: "action", round: "flop", player: 1, pot: 100, has_children: true }
      ]))
    ]
  },
  estimate: {
    title: "Estimate memory",
    blurb: "Estimate is answered on the event-loop thread. It does not start a worker.",
    steps: [
      ui(1, "Button click", "The GUI sends ranges and board. It does not copy the whole form first; the server estimates the current tree.", "mainwindow.cpp", null),
      ui(2, "One call, then wait", "estimateMemory uses call(), so the UI thread blocks until the reply. There is no status poll.", "remote_backend.cpp", req("estimate", { range_ip: "AA", range_oop: "KK", board: "Qs,Jh,2h" })),
      server(3, "Refuse while busy", "estimate sits after the second busy check, so it waits out load, build, solve and save.", "src/server/main.cpp", null),
      server(4, "Count on the current tree", "LocalBackend::estimateMemory calls PokerSolver::estimate_tree_memory and returns a long long.", "local_backend.cpp", null),
      reply(5, "Byte count", "The GUI multiplies by 4, then by the isomorphism and half-float factors, and shows MB or GB.", null, ok(1843200))
    ]
  },
  save: {
    title: "Save strategy",
    blurb: "The path is a path on the machine running the server.",
    steps: [
      ui(1, "Pick a file", "The JSON action reads dump_round from settings and calls saveStrategy on the worker thread.", "mainwindow.cpp", null),
      ui(2, "Send path and rounds", "args are only path and dump_rounds. The solved tree already lives in the server process.", "remote_backend.cpp", req("save", { path: "C:/solver/output_strategy.json", dump_rounds: 2 })),
      server(3, "Write on the worker", "startJob(\"save\") calls dump_strategy. Progress lines say which street depth is being written.", "local_backend.cpp", null),
      reply(4, "Accepted", "result is null. The file does not exist yet.", null, ok(null)),
      ui(5, "Wait for the worker", "The same 300 ms status loop used by build and solve runs until busy is false.", "remote_backend.cpp", req("status")),
      reply(6, "File written", "The server machine now has the JSON. A remote GUI does not receive the file bytes.", null, statusReply(false, "save", true, [
        { iteration: 0, exploitability: 0, message: "Saving done." }
      ]))
    ]
  },
  cache: {
    title: "Cached session info",
    blurb: "Ranks, deck, board and stack would be fetched on every table cell. They are cached instead.",
    steps: [
      ui(1, "Table asks for ranks", "rowCount, columnCount and every cell call getRanks(). RemoteBackend::info() serves the cache when it is valid.", "tablestrategymodel.cpp", null),
      ui(2, "First miss sends info", "One command returns ranks, deck, board and stack together. Later calls do not open a socket.", "remote_backend.cpp", req("info")),
      server(3, "Read the session", "info() is allowed during solve. It is refused while load or build is replacing the tree.", "src/server/main.cpp", null),
      reply(4, "Four fields", "RemoteBackend stores them under a mutex and sets info_valid_.", null, ok({
        ranks: ["A", "K", "Q", "J", "T", "9", "8", "7", "6", "5", "4", "3", "2"],
        deck: ["2c", "2d", "2h", "2s"],
        board: "Qs,Jh,2h",
        stack: 250
      })),
      ui(5, "Cache cleared on the next job", "load, build and solve call invalidateInfo() before sending, so the next paint refetches.", "remote_backend.cpp", null)
    ]
  }
};

function ui(n, title, text, file, json) { return { lane: "ui", n, title, text, file, json, dir: json ? "to-server" : null }; }
function server(n, title, text, file, json) { return { lane: "server", n, title, text, file, json, dir: null }; }
function reply(n, title, text, file, json) { return { lane: "reply", n, title, text, file, json, dir: "to-ui" }; }
function req(cmd, args) { return { cmd, args: args || {} }; }
function ok(result) { return { ok: true, result }; }
function statusReply(busy, job, loaded, events) {
  return { ok: true, result: { busy, job, loaded, error: "", events: events || [] } };
}
function sampleRequest() {
  return {
    board: "Qs,Jh,2h",
    oop_commit: 25,
    ip_commit: 25,
    stack: 250,
    current_round: 1,
    raise_limit: 3,
    range_ip: "AA,KK",
    range_oop: "QQ,JJ",
    max_iteration: 100,
    accuracy: 0.5,
    thread_number: 8,
    tree_settings: { flop_ip: { bet_sizes: [50], raise_sizes: [60], allin: true } }
  };
}

const list = document.getElementById("flows");
const title = document.getElementById("flow-title");
const blurb = document.getElementById("flow-blurb");
const cols = { ui: document.getElementById("ui-steps"), server: document.getElementById("server-steps"), reply: document.getElementById("reply-steps") };
const jsonEl = document.getElementById("json");
const codeEl = document.getElementById("code");
const who = document.getElementById("who");
const where = document.getElementById("where");
const packetOut = document.getElementById("packet-out");
const packetBack = document.getElementById("packet-back");

let current = "solve";
let cursor = -1;
let timer = null;

Object.entries(flows).forEach(([id, flow]) => {
  const b = document.createElement("button");
  b.className = "flow-btn";
  b.innerHTML = `${flow.title}<small>${flow.steps.length} steps</small>`;
  b.onclick = () => select(id);
  list.appendChild(b);
});

function select(id) {
  stop();
  current = id;
  cursor = -1;
  [...list.children].forEach((b, i) => b.classList.toggle("active", Object.keys(flows)[i] === id));
  title.textContent = flows[id].title;
  blurb.textContent = flows[id].blurb;
  render(id);
  showStep(null);
}

function render(id) {
  Object.values(cols).forEach(el => el.innerHTML = "");
  flows[id].steps.forEach((step, index) => {
    const card = document.createElement("article");
    card.className = "step";
    card.dataset.index = index;
    card.innerHTML = `<div class="n">STEP ${step.n}</div><h3>${step.title}</h3><p>${step.text}</p>`;
    card.onclick = () => { stop(); focus(index); };
    cols[step.lane].appendChild(card);
  });
}

function focus(index) {
  cursor = index;
  const steps = flows[current].steps;
  document.querySelectorAll(".step").forEach(card => {
    const i = Number(card.dataset.index);
    card.classList.toggle("on", i <= index);
    card.classList.toggle("current", i === index);
  });
  const step = steps[index];
  const laneName = step.lane === "ui" ? "UI sends" : step.lane === "server" ? "Inside the server" : "Server response";
  const source = (stepSource[current] && stepSource[current][step.n]) || null;
  who.textContent = `Step ${step.n} · ${laneName}`;
  where.textContent = source ? source.file : (step.file || "");
  codeEl.textContent = source ? source.code : "";
  jsonEl.textContent = step.json ? JSON.stringify(step.json, null, 2) : "No socket message on this step. Work stays inside the process.";
  pulse(step.dir);
  document.querySelector(`.step[data-index="${index}"]`)?.scrollIntoView({ block: "nearest" });
}

function showStep(step) {
  if (!step) {
    who.textContent = "Press Play";
    where.textContent = "Each highlighted card is one step. Source is the C++ for that step. Wire JSON is the line on the socket, when that step sends or receives one.";
    codeEl.textContent = "";
    jsonEl.textContent = "Messages are one JSON object plus a newline.\nRequest: {\"cmd\",\"args\"}\\n\nReply: {\"ok\":true,\"result\":...}\\n";
  }
}

function pulse(dir) {
  [packetOut, packetBack].forEach(p => { p.className = "packet"; });
  const node = dir === "to-server" ? packetOut : dir === "to-ui" ? packetBack : null;
  if (!node) return;
  node.classList.add("show", dir);
}

function play() {
  stop();
  const total = flows[current].steps.length;
  if (cursor >= total - 1) cursor = -1;
  timer = setInterval(() => {
    if (cursor >= total - 1) { stop(); return; }
    focus(cursor + 1);
  }, Number(document.getElementById("speed").value));
}
function stop() { clearInterval(timer); timer = null; }
function stepOnce() {
  stop();
  const total = flows[current].steps.length;
  focus(Math.min(cursor + 1, total - 1));
}
function stepBack() {
  stop();
  if (cursor <= 0) {
    focus(0);
    return;
  }
  focus(cursor - 1);
}
function reset() { select(current); }

document.getElementById("play").onclick = play;
document.getElementById("pause").onclick = stop;
document.getElementById("prev").onclick = stepBack;
document.getElementById("step").onclick = stepOnce;
document.getElementById("reset").onclick = reset;

select("startup");
