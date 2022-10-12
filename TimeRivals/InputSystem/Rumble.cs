using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.DualShock;
using System.Linq;

public class Rumble : MonoBehaviour
{
    private PlayerInput _playerInput;

    [Header("Rumble Settings")]
    [SerializeField] private float _rumbleDurrationShort = 0.1f;
    [SerializeField] private float _rumbleDurrationLong = 0.35f;
    [SerializeField] private float _low = 1f;
    [SerializeField] private float _high = 2f;

    private void Awake()
    {
        _playerInput = GetComponent<PlayerInput>();

        var gamepad = GetGamepad();

        if (gamepad != null)
        {
            gamepad.SetMotorSpeeds(1, 2);
        }
        Invoke(nameof(StopRumble), 0.1f);
    }

    private Gamepad GetGamepad()
    {
        return Gamepad.all.FirstOrDefault(g => _playerInput.devices.Any(d => d.deviceId == g.deviceId));
    }
    private DualShockGamepad GetDualShockGamepad()
    {
        return (DualShockGamepad)DualShockGamepad.all.FirstOrDefault(g => _playerInput.devices.Any(d => d.deviceId == g.deviceId));
    }

    private void StopRumble()
    {

        var gamepad = GetGamepad();
        if (gamepad != null)
        {
            gamepad.SetMotorSpeeds(0, 0);
        }
    }

    public void ShortRumble()
    {

        var gamepad = GetGamepad();
        if (gamepad != null)
        {
            gamepad.SetMotorSpeeds(_low, _high);
        }
        Invoke(nameof(StopRumble), _rumbleDurrationShort);
    }

    public void LongRumble()
    {

        var gamepad = GetGamepad();
        if (gamepad != null)
        {
            gamepad.SetMotorSpeeds(_low, _high);
        }
        Invoke(nameof(StopRumble), _rumbleDurrationLong);
    }

}
