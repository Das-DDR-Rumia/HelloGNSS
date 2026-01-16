import os
import subprocess
import questionary
from rich.console import Console
from rich.panel import Panel
import typer
from pathlib import Path

app = typer.Typer()
console = Console()

# All available actions with descriptions
TASKS = [
    {
        "name": "init-build-dir",
        "desc": "Create build directory and initialize CMake configuration."
    },
    {
        "name": "build-all",
        "desc": "Build all targets using CMake."
    },
    {
        "name": "build-target",
        "desc": "Build a specific target using CMake."
    },
    {
        "name": "clean-all",
        "desc": "Clean all build artifacts."
    },
    {
        "name": "clean-target",
        "desc": "Clean a specific target."
    },
    {
        "name": "run-tests",
        "desc": "Run all tests using CTest."
    },
    {
        "name": "configure",
        "desc": "Configure CMake project with specific options."
    },
    {
        "name": "exit",
        "desc": "Exit the CLI tool."
    }
]

# Default build directory
BUILD_DIR = "build"

def ensure_build_dir(build_dir=BUILD_DIR):
    """Ensure build directory exists."""
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
        return True
    return False

def run_cmake_command(command, build_dir=BUILD_DIR):
    """Run a CMake command."""
    try:
        subprocess.run(command, cwd=build_dir, check=True)
        return True
    except subprocess.CalledProcessError:
        console.print("[red]Command failed.[/red]")
        return False

def init_build_dir():
    """Initialize build directory and run CMake configuration."""
    created = ensure_build_dir()
    if created:
        console.print(f"[green]Created build directory: {BUILD_DIR}[/green]")
    
    console.print(Panel.fit("[cyan]Running: cmake ..[/cyan]"))
    if run_cmake_command(["cmake", ".."]):
        console.print("[green]CMake configuration completed successfully.[/green]")
    else:
        console.print("[red]CMake configuration failed.[/red]")

def build_all():
    """Build all targets using CMake."""
    ensure_build_dir()
    console.print(Panel.fit("[cyan]Running: cmake --build .[/cyan]"))
    if run_cmake_command(["cmake", "--build", "."]):
        console.print("[green]Build completed successfully.[/green]")

def build_target():
    """Build a specific target using CMake."""
    ensure_build_dir()
    
    # Get available targets
    result = subprocess.run(
        ["cmake", "--build", ".", "--target", "help"], 
        cwd=BUILD_DIR, 
        capture_output=True, 
        text=True
    )
    
    if result.returncode != 0:
        console.print("[red]Failed to get available targets.[/red]")
        return
    
    # Parse targets from output
    lines = result.stdout.strip().split("\n")
    targets = []
    for line in lines:
        if "..." in line:
            target = line.split("...")[0].strip()
            if target and target != "all" and not target.startswith("The following"):
                targets.append(target)
    
    if not targets:
        console.print("[yellow]No specific targets found.[/yellow]")
        return
    
    # Let user select target
    selected = questionary.select(
        "Choose a target to build:",
        choices=targets
    ).ask()
    
    if selected:
        console.print(Panel.fit(f"[cyan]Running: cmake --build . --target {selected}[/cyan]"))
        if run_cmake_command(["cmake", "--build", ".", "--target", selected]):
            console.print(f"[green]Target '{selected}' built successfully.[/green]")

def clean_all():
    """Clean all build artifacts."""
    if not os.path.exists(BUILD_DIR):
        console.print("[yellow]Build directory doesn't exist. Nothing to clean.[/yellow]")
        return
    
    console.print(Panel.fit("[cyan]Running: cmake --build . --target clean[/cyan]"))
    if run_cmake_command(["cmake", "--build", ".", "--target", "clean"]):
        console.print("[green]All targets cleaned successfully.[/green]")

def clean_target():
    """Clean a specific target."""
    if not os.path.exists(BUILD_DIR):
        console.print("[yellow]Build directory doesn't exist. Nothing to clean.[/yellow]")
        return
    
    # Ask for target name
    target = questionary.text("Enter the target name to clean:").ask()
    
    if target:
        clean_cmd = ["cmake", "--build", ".", "--target", f"clean_{target}"]
        console.print(Panel.fit(f"[cyan]Running: {' '.join(clean_cmd)}[/cyan]"))
        
        try:
            if run_cmake_command(clean_cmd):
                console.print(f"[green]Target '{target}' cleaned successfully.[/green]")
        except Exception:
            console.print(f"[red]Failed to clean target '{target}'. Make sure the target exists.[/red]")

def run_tests():
    """Run all tests using CTest."""
    ensure_build_dir()
    console.print(Panel.fit("[cyan]Running: ctest -V[/cyan]"))
    if run_cmake_command(["ctest", "-V"]):
        console.print("[green]Tests completed successfully.[/green]")
    else:
        console.print("[red]Some tests failed.[/red]")

def configure():
    """Configure CMake project with specific options."""
    build_type = questionary.select(
        "Select build type:",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"]
    ).ask()
    
    options = []
    while questionary.confirm("Add another CMake option?").ask():
        option_name = questionary.text("Option name:").ask()
        option_value = questionary.text("Option value:").ask()
        options.append(f"-D{option_name}={option_value}")
    
    ensure_build_dir()
    cmd = ["cmake", f"-DCMAKE_BUILD_TYPE={build_type}"] + options + [".."]
    console.print(Panel.fit(f"[cyan]Running: {' '.join(cmd)}[/cyan]"))
    
    if run_cmake_command(cmd):
        console.print("[green]CMake configuration updated successfully.[/green]")

def main_menu():
    """Main loop for interactive selection and task execution."""
    while True:
        options = [
            questionary.Choice(
                title=f"{task['name']} - {task['desc']}",
                value=task["name"]
            ) for task in TASKS
        ]

        selected = questionary.select(
            "Choose a task to perform:",
            choices=options
        ).ask()

        if selected == "exit":
            console.print("[bold blue]Exiting. Goodbye![/bold blue]")
            break

        confirm = questionary.confirm(f"Proceed with `{selected}`?").ask()

        if confirm:
            if selected == "init-build-dir":
                init_build_dir()
            elif selected == "build-all":
                build_all()
            elif selected == "build-target":
                build_target()
            elif selected == "clean-all":
                clean_all()
            elif selected == "clean-target":
                clean_target()
            elif selected == "run-tests":
                run_tests()
            elif selected == "configure":
                configure()
        else:
            console.print("[yellow]Cancelled. Returning to menu.[/yellow]")

@app.command()
def interactive():
    """Start the interactive CLI."""
    console.print("[bold green]Welcome to the CMake Project CLI Tool![/bold green]")
    main_menu()

if __name__ == "__main__":
    app()

