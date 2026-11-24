(function () {
    var root = document.documentElement;
    var saved = null;
    try {
        saved = localStorage.getItem("blood-doc-theme");
    } catch (e) {}

    var prefersDark =
        window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches;
    var theme = saved || (prefersDark ? "dark" : "light");

    function applyTheme(value) {
        root.setAttribute("data-theme", value);
        var button = document.getElementById("theme-toggle");
        if (button) {
            button.textContent = value === "dark" ? "Light" : "Dark";
        }
    }

    applyTheme(theme);

    var themeButton = document.getElementById("theme-toggle");
    if (themeButton) {
        themeButton.addEventListener("click", function () {
            var next = root.getAttribute("data-theme") === "dark" ? "light" : "dark";
            try {
                localStorage.setItem("blood-doc-theme", next);
            } catch (e) {}
            applyTheme(next);
        });
    }

    var privateToggle = document.getElementById("show-private");
    var showPrivate = true;
    try {
        var storedPrivate = localStorage.getItem("blood-doc-private");
        if (storedPrivate !== null) {
            showPrivate = storedPrivate === "1";
        }
    } catch (e) {}

    if (privateToggle) {
        privateToggle.checked = showPrivate;
        document.body.classList.toggle("show-private", showPrivate);
        privateToggle.addEventListener("change", function () {
            document.body.classList.toggle("show-private", privateToggle.checked);
            try {
                localStorage.setItem(
                    "blood-doc-private",
                    privateToggle.checked ? "1" : "0"
                );
            } catch (e) {}
        });
    }

    var expandAll = document.getElementById("expand-all");
    if (expandAll) {
        expandAll.addEventListener("click", function () {
            document.querySelectorAll("details.item").forEach(function (item) {
                item.open = true;
            });
        });
    }

    var collapseAll = document.getElementById("collapse-all");
    if (collapseAll) {
        collapseAll.addEventListener("click", function () {
            document.querySelectorAll("details.item").forEach(function (item) {
                item.open = false;
            });
        });
    }
})();
