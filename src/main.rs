extern crate gtk;

use gtk::prelude::*;

use gtk::{Window, WindowType, FlowBox, Button, Scale, Orientation};

fn main() {
    if gtk::init().is_err() {
        println!("Failed to initialize GTK.");
        return;
    }

    let window = Window::new(WindowType::Toplevel);
    window.set_title("aburndance");
    window.set_default_size(350, 70);

    let main_container = FlowBox::new();
    window.add(&main_container);

    let mode_buttons_container = FlowBox::new();
    let prev_mode_button = Button::new_with_label("Mode -");
    let next_mode_button = Button::new_with_label("Mode +");
    mode_buttons_container.insert(&prev_mode_button, 0);
    mode_buttons_container.insert(&next_mode_button, 1);
    main_container.insert(&mode_buttons_container, 0);

    let param_buttons_container = FlowBox::new();
    let prev_param_button = Button::new_with_label("Param -");
    let next_param_button = Button::new_with_label("Param +");
    param_buttons_container.insert(&prev_param_button, 0);
    param_buttons_container.insert(&next_param_button, 1);
    main_container.insert(&param_buttons_container, 1);

    let param_scale = Scale::new_with_range(Orientation::Horizontal, 0.0, 1.0, 0.01);
    window.add(&param_scale);
    main_container.insert(&param_scale, 2);

    window.show_all();

    window.connect_delete_event(|_, _| {
        gtk::main_quit();
        Inhibit(false)
    });

    prev_mode_button.connect_clicked(|_| {
        println!("previous mode!");
    });

    next_mode_button.connect_clicked(|_| {
        println!("next mode!");
    });

    prev_param_button.connect_clicked(|_| {
        println!("previous param!");
    });

    prev_param_button.connect_clicked(|_| {
        println!("previous param!");
    });

    gtk::main();
}
