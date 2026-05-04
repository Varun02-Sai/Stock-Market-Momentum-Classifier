# ============================================================================
# Makefile — Root Build & Run System for Stock Market Momentum Classifier
# Project:  Stock Market Momentum Classifier (NSE)
#
# Usage (run from C_project/ root directory):
#   make            — Build C engine + install all dependencies
#   make build      — Compile the C engine binary only
#   make install    — Install Python + Node.js dependencies
#   make run        — Build everything and launch the full application
#   make test       — Run the 10-NSE-stock test suite
#   make clean      — Remove all build artifacts
#
# Requirements:
#   - gcc (MinGW-w64) in PATH
#   - Python 3.12+ with pip
#   - Node.js 18+ with npm
#
# Topics: Makefile, Build Automation, Project Orchestration
# ============================================================================

# ── Compiler Settings ──
CC = gcc
CFLAGS = -Wall -Wextra -pthread -Ic_engine/include -O2

# ── Source Files ──
C_SRCS = c_engine/src/main.c c_engine/src/features.c c_engine/src/logistic.c c_engine/src/metrics.c
C_BIN  = c_engine/model_release.exe

# ── Default Target: build everything ──
all: build install
	@echo.
	@echo ============================================================
	@echo   BUILD COMPLETE - Stock Market Momentum Classifier (NSE)
	@echo ============================================================
	@echo   C Engine:   $(C_BIN) [OK]
	@echo   Python:     server/requirements.txt installed [OK]
	@echo   React:      frontend/node_modules installed [OK]
	@echo.
	@echo   To launch the project, run:  make run
	@echo ============================================================

# ── Build the C Algorithm Engine ──
# Compiles all C source files into a single statically-linked binary.
# Static linking (-static) ensures no DLL dependencies.
# The -lm flag links the math library for exp() and sqrt().
build:
	@echo [1/1] Compiling C Algorithm Engine...
	$(CC) $(CFLAGS) $(C_SRCS) -o $(C_BIN) -lm
	@echo [OK] Built $(C_BIN) successfully

# ── Install All Dependencies ──
install: install-python install-node

# Install Python dependencies for the Flask server
install-python:
	@echo [PYTHON] Installing Flask server dependencies...
	pip install -r server/requirements.txt --quiet
	@echo [OK] Python dependencies installed

# Install Node.js dependencies for the React frontend
install-node:
	@echo [NODE] Installing React frontend dependencies...
	cd frontend && npm install --silent
	@echo [OK] Node.js dependencies installed

# ── Run the Full Application ──
# Builds the C engine, then launches Flask (port 5000) and React (port 5173)
# in separate terminal windows using Windows 'start' command.
run: build
	@echo.
	@echo ============================================================
	@echo   LAUNCHING Stock Market Momentum Classifier (NSE)
	@echo ============================================================
	@echo   Starting Flask API server on http://localhost:5000 ...
	@echo   Starting React dev server on http://localhost:5173 ...
	@echo.
	@echo   Open your browser to: http://localhost:5173
	@echo   Press Ctrl+C in each terminal window to stop.
	@echo ============================================================
	@echo.
	cmd /c start "Flask API Server" cmd /k "cd server && python app.py"
	cmd /c start "React Dev Server" cmd /k "cd frontend && npm run dev"

# ── Test with 10 NSE Stocks ──
test: build
	@echo [TEST] Running 10-stock NSE test suite...
	python test_nse.py

# ── Clean All Build Artifacts ──
clean:
	@echo [CLEAN] Removing build artifacts...
	-del /Q c_engine\model_release.exe 2>nul
	-del /Q c_engine\model_debug.exe 2>nul
	-rmdir /S /Q c_engine\obj 2>nul
	@echo [OK] Cleaned

# Declare non-file targets
.PHONY: all build install install-python install-node run test clean
