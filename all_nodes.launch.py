from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_cpp_pkg',
            executable='camera_node',
            name='camera_node',
            output='screen'
        ),
        Node(
            package='my_cpp_pkg',
            executable='uart_node',
            name='uart_node',
            output='screen'
        ),
        Node(
            package='my_cpp_pkg',
            executable='tcp_server_node',
            name='tcp_server_node',
            output='screen'
        ),
    ])
