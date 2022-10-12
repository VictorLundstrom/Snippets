using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ActiveRagdoll;
using UnityEngine.UI;
public class PlayerMovementBehaviour : MonoBehaviour
{
    [Header("Component References")]
    public Rigidbody playerRigidbody;
    public Transform ragdollTransform;
    public Transform animatorTransform;

    [Header("Movement Settings")]
    public float movementSpeed = 5.0f;
    public float turnSpeed = 0.1f;

    [Header("Jump Settings")]
    public float jumpAmount = 550f;
    public float gravityScale = 5;
    public PlayerGroundCheck _playerGroundCheck; 

    [Header("Grab Settings")]
    public float grabRange = 0.5f;
    public float _knocback = 600.0f;
    private Rigidbody currentObjectRigidbody;
    private Collider currentObjectCollider;
    private bool _physicalObjectMultiple;
    private const float _delayTimer = 0.16f;
    private float _currDelayTime = 0;
    public bool PhysicalObjectMultiple { get { return _physicalObjectMultiple; } set { _physicalObjectMultiple = value; } }

    [Header("CanvasIndicator")]
    public GameObject CanvasIndicator;

    private string[] swordSwings = new string[6] { "event:/Master/Weapons/Sword/Sword Swing 1", "event:/Master/Weapons/Sword/Sword Swing 2", "event:/Master/Weapons/Sword/Sword Swing Low", "event:/Master/Weapons/Sword/Sword Swing Low 2", "event:/Master/Weapons/Sword/Sword Swing Low 3", "event:/Master/Weapons/Sword/Sword Swing Low 4" };
    private string[] blunderbussReload = new string[3] { "event:/Master/Weapons/Gun/Magazine reload", "event:/Master/Weapons/Gun/Magazine reload 2", "event:/Master/Weapons/Gun/Magazine reload 3" };

    //GrabbedIndicator
    [SerializeField] private GameObject _grabbedIndicator;
   
    public GameObject GrabbedIndicator { get { return _grabbedIndicator; } }

    [SerializeField] private Image _grabbedIndicatorFill;
    //PickupIndicator
    public GameObject _pickupIndicator;
    public GameObject _pickupImageFill;

    [Header("Particles")]
    public ParticleSystem WeaponDestroy;
    public ParticleSystem DustWalking;
    public ParticleSystem Jumping;

    private int _grabbedByPlayerID = -1;
    public bool _CanPress = false;
    public int GrabbedByPlayerID { get { return _grabbedByPlayerID; } set { _grabbedByPlayerID = value; } }
    [SerializeField] private int _requiredButtonPresses;
    public int RequiredButtonPresses { get { return _requiredButtonPresses; } set { _requiredButtonPresses = value; } }
    private int _currGrabbedButtonCounter;

    private bool _isGrabPressed;
    public bool IsGrabPressed { get { return _isGrabPressed; } set { _isGrabPressed = value; } }
    public GripModule gripModule;

    private Camera mainCamera;
    private Vector3 movementDirection;
    private Vector3 rotationDirection;

    /////////////////////////////////////////////

    [SerializeField] private GameObject _currWeapon = null;
    [SerializeField] private Transform _rightHandTransform;
    [SerializeField] private PlayerController _playerController; //Ref to PlayerController
    [SerializeField] private HealthSystem _healthSystem; //Ref to HealthSystem
    //pickup
    private WeaponPickup _nearbyWeaponScript; //Script that is attached to all nearby weapons
    public WeaponPickup NearbyWeaponScript { get { return _nearbyWeaponScript; } set { _nearbyWeaponScript = value; } }
    private bool _isPickupPressed; //callback value
    [SerializeField] private float _pickupTime = 1.0f; //How long it takes to loot item
    private float _currPickupTime = 0.0f;
    public float CurrPickupTime { get { return _currPickupTime; } }

    //attack
    private AttackIndexes _currAttack = AttackIndexes.NONE; //Default Startattack
    public AttackIndexes CurrAttack { get { return _currAttack; } }

    private bool _startAttack;
    public bool StartAttack { get { return _startAttack; } set { _startAttack = value; } }

    private bool _currentlyAttacking;
    public bool CurrentlyAttacking { get { return _currentlyAttacking; } set { _currentlyAttacking = value; } }

    //Animation
    [SerializeField] private PlayerAnimationBehaviour _playerAnimationBehavior;

    //Playerstate
    private PlayerStates _playerState = PlayerStates.MOVEMENT; //Default state
    public PlayerStates PlayerState { get { return _playerState; } set { _playerState = value; } }

    private void Awake()
    {
        mainCamera = CameraManager.Instance.GetGameplayCamera();
            transform.position = GameHandler.instance.PlayerSpawnLocations.SpawnPoints[_playerController.PlayerID]; //Initial Spawn for player
    }
    public void UpdateMovementData(Vector3 newMovementDirection)
    {
        movementDirection = newMovementDirection;
    } //UpdateMovementData()

    public void UpdateRotationData(Vector3 newRotationDirection)
    {
        rotationDirection = newRotationDirection;
    } //UpdateRotationData()

    public void UpdateJumpData() //Also used for Grabbed State
    {
        if (_playerState == PlayerStates.MOVEMENT)
        {
            PlayerJump();
        }
        if (_playerState == PlayerStates.GRABBED)
        {
            _currGrabbedButtonCounter++;
            _grabbedIndicatorFill.fillAmount += 1 / (float)_requiredButtonPresses;
        }
    }

    public void HandlePrinterButton(bool b)
    {
        _CanPress = b;
    }

    public void UpdateGrabData(bool pressedGrab)
    {
        if (pressedGrab && _currAttack == AttackIndexes.NONE) //If player started pressing GrabButton and does not have a weapon equiped
        {
            if (_playerState == PlayerStates.MOVEMENT) //If player is in Movementstate AND grab is NOT on cooldown.
            {
                Grab();
            }
        }
        else if (_playerState == PlayerStates.GRAB) //If player stopped pressing GrabButton AND is currently in Grab State.
        {
            UnGrab();
        }
    }

    private void Grab()
    {
        _playerAnimationBehavior.ToggleGrabAnimation(true);
        _playerState = PlayerStates.GRAB;
    }
    private void UnGrab()
    {
        gripModule.GrippDisable();
        _playerAnimationBehavior.ToggleGrabAnimation(false);
        _playerState = PlayerStates.MOVEMENT;
    }

    public void UpdateGrabbedState(bool doGrab, int playerID)
    {
        if (doGrab) //If you got grabbed
        {
            InterruptPlayerState(PlayerStates.GRAB);
            _grabbedByPlayerID = playerID;
            _grabbedIndicator.SetActive(true);

        }
        else if (_playerState == PlayerStates.GRABBED) //If you got released
        {
            //Reset all neccesary variables
            _grabbedIndicatorFill.fillAmount = 0;
            _grabbedIndicator.SetActive(false);
            _playerAnimationBehavior.ToggleGrabbedAnimation(false);
            _currGrabbedButtonCounter = 0;
            _grabbedByPlayerID = -1;
            RestoreCurrWeapon();

            _playerState = PlayerStates.MOVEMENT;
        }


    }
    public void PlayerJump()
    {
        if (_playerGroundCheck.IsGrounded)
        {
            playerRigidbody.AddForce(Vector3.up * jumpAmount, ForceMode.Impulse);
            Jumping.Play();
            _playerAnimationBehavior.PlayJumpAnimation(); 
        }
    }

    public void UpdatePickupData(bool isPickupPressed)
    {
        _isPickupPressed = isPickupPressed;
        if (_isPickupPressed && _playerState == PlayerStates.MOVEMENT)
        {
            //If we are NOT in pickup range OR if the weapon is being picked up by another player OR if we already have a weapon equipped
            if (!_nearbyWeaponScript || (_nearbyWeaponScript.BeingLootedByPlayerID != -1 && _nearbyWeaponScript.BeingLootedByPlayerID != _playerController.PlayerID) || _currWeapon)
            {
                return;
            }

            _nearbyWeaponScript.BeingLootedByPlayerID = _playerController.PlayerID;
            _playerAnimationBehavior.TogglePickupAnimation(true); //Start Pickup animation
            _pickupIndicator.SetActive(true);
            _pickupImageFill.SetActive(true);
            _playerState = PlayerStates.PICKUP;
        }

    }

    public void UpdateDestroyData() //When player discards their weapon
    {
        if (_playerState == PlayerStates.MOVEMENT)
        {
            DestroyWeapon();
        }
    }

    public void UpdateAttackData(bool isAttackPressed)
    {
        if (!_currWeapon) //If we don't have a weapon equiped
            return;

        if (isAttackPressed && _playerState == PlayerStates.MOVEMENT) //If AttackButton is pressed and we are NOT currently attacking
        {
            switch (_currAttack)
            {
                case AttackIndexes.BAT:
                case AttackIndexes.SWORD:
                    _playerAnimationBehavior.PlayWindupAnimation(); //Start the Windup animation for Sword/Bat-Attack.
                    _playerState = PlayerStates.ATTACK;
                    break;

                case AttackIndexes.RIFLE:
                    if (_playerAnimationBehavior.animationAnimator.GetCurrentAnimatorStateInfo(2).IsName("RifleGrip")) //This check is neccesary, because if you fire as soon as you pick up rifle the bullet will spawn inside of rifle
                    {
                        _currWeapon.GetComponent<WeaponBlunderbuss>().Fire();
                        _playerAnimationBehavior.PlayRifleFireAnimation(); //Start the Recoil animation for Rifle
                        _playerState = PlayerStates.ATTACK;
                    }
                    break;
                case AttackIndexes.CROSSBOW:
                    if (_playerAnimationBehavior.animationAnimator.GetCurrentAnimatorStateInfo(2).IsName("RifleGrip")) //This check is neccesary, because if you fire as soon as you pick up rifle the bullet will spawn inside of rifle
                    {
                        _currWeapon.GetComponent<WeaponCrossbow>().Fire();
                        _playerAnimationBehavior.PlayRifleFireAnimation(); //Start the Recoil animation for Rifle
                        _playerState = PlayerStates.ATTACK;
                    }
                    break;

                case AttackIndexes.MACEFISH:
                case AttackIndexes.MALLET: //Play SmashWindup animation
                    _playerAnimationBehavior.PlaySmashWindupAnimation(); //Start the Smash Windup animation for MalletAttacks.
                    _playerState = PlayerStates.ATTACK;
                    break;
                default:
                    Debug.Log("Error at UpdateAttackData SwitchCase");
                    break;
            }
            Debug.Log($"Type of Attack started = {System.Enum.GetName(typeof(AttackIndexes), _currAttack)}");

        }
    }

    private void FixedUpdate()
    {
        playerRigidbody.AddForce(Physics.gravity * (gravityScale - 1) * playerRigidbody.mass); // Applying a gravityforce to the player

        switch (_playerState)
        {
            case PlayerStates.MOVEMENT:
                MovePlayer();
                break;
            case PlayerStates.JUMP:
                MovePlayer();
                break;
            case PlayerStates.GRAB:
                MovePlayer();
                break;
            case PlayerStates.ATTACK:
                MovePlayer();
                RotatePlayer();
                HandleAttack();
                break;
            case PlayerStates.GRABBED: //Used in Update
                break;
            case PlayerStates.PICKUP: //Used in Update
                break;
            case PlayerStates.DEAD: //Used in Update
                break;
            default:
                Debug.Log("Error at FixedUpdate PlayerStateSwitchCase");
                break;
        }
    } //FixedUpdate()

    private void Update()
    {
        if ( _playerState != PlayerStates.GRABBED && gravityScale < 40 && !_playerGroundCheck.IsGrounded ) // resets the gravity if the float bug appears
        {
                gravityScale = 40;
        }

        switch (_playerState)
        {
            case PlayerStates.PICKUP:
                HandlePickup();
                break;
            case PlayerStates.GRAB:
                
                if (_physicalObjectMultiple)
                    return;
                

                RotatePlayer();
                if(_currDelayTime > _delayTimer)
                {
                    gripModule.GrippEnable();
                    _currDelayTime = 0;
                }
                    _currDelayTime += Time.deltaTime;
                break;
            case PlayerStates.GRABBED:
                HandleGrabbed();
                break;
            case PlayerStates.MOVEMENT: //Used in FixedUpdate
                RotatePlayer();
                break;
            case PlayerStates.JUMP: //Used in FixedUpdate
                RotatePlayer();
                break;
            case PlayerStates.ATTACK: //Used in FixedUpdate
                break;
            case PlayerStates.DEAD:
                break;
            default:
                Debug.Log("Error at Update PlayerStateSwitchCase");
                break;
        }

    } //Update()


    private void MovePlayer()
    {
        Vector3 movement = CameraDirection(movementDirection) * movementSpeed;
        playerRigidbody.velocity = movement; 

        if(playerRigidbody.velocity.magnitude <= 1)
        {
            DustWalking.Stop();
        }

    }

    private void RotatePlayer()
    {
        if (rotationDirection == Vector3.zero)
            return;
        Quaternion rotation = Quaternion.Slerp(playerRigidbody.rotation, Quaternion.LookRotation(CameraDirection(rotationDirection)), turnSpeed);
        ragdollTransform.rotation = rotation;
        animatorTransform.rotation = rotation;
    }

    private void HandlePickup()
    {
        if (!_nearbyWeaponScript || !_isPickupPressed) //if we are out of range of pickup OR we stopped holding Pickupbutton
        {
            _currPickupTime = 0.0f;
            _playerAnimationBehavior.TogglePickupAnimation(false); //Stop Pickup animation
            _pickupIndicator.SetActive(false);
            _pickupImageFill.SetActive(false);
            if (_nearbyWeaponScript)
            {
                if(_nearbyWeaponScript.BeingLootedByPlayerID == _playerController.PlayerID)
                {
                    _nearbyWeaponScript.BeingLootedByPlayerID = -1;
                }
            }

            _playerState = PlayerStates.MOVEMENT; //Go back to Movement State
            return;
        }

        if (_currPickupTime >= _pickupTime) //if player successfully picked up weapon
        {
            PickupWeapon();
            _currPickupTime = 0.0f;
            _playerAnimationBehavior.TogglePickupAnimation(false); //Stop Pickup animation
            RestoreCurrWeapon();
            _pickupIndicator.SetActive(false);
            _pickupImageFill.SetActive(false);

            _playerState = PlayerStates.MOVEMENT;//Go back to Movement State
        }

        _currPickupTime += Time.deltaTime;
    }


    private void HandleAttack()
    {

        AnimatorStateInfo stateInfo = _playerAnimationBehavior.animationAnimator.GetCurrentAnimatorStateInfo(2);

        switch (_currAttack)
        {
            case AttackIndexes.BAT:
            case AttackIndexes.SWORD: //If we're attacking with a Sword/Bat
                _currWeapon.transform.GetChild(0).gameObject.SetActive(true); //VFX Enable

                if (stateInfo.IsName("Windup") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && !_currentlyAttacking) //If Attack animation started
                {
                    int randNr = Random.Range(0, swordSwings.Length);
                    FMODUnity.RuntimeManager.PlayOneShot(swordSwings[randNr], transform.position);
                    
                    _currentlyAttacking = true;
                    _currWeapon.layer = _currWeapon.GetComponent<WeaponSword>().AttackWeaponLayer; //Change layer on weapon so we can hit players/physical objects
                }

                if (stateInfo.IsName("Attack") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && _currentlyAttacking)//if Attack animation ended
                {
                    _currentlyAttacking = false;
                    _currWeapon.transform.GetChild(0).gameObject.SetActive(false); //VFX Disable
                    _currWeapon.layer = _currWeapon.GetComponent<WeaponSword>().DefaultWeaponLayer; //Change layer on weapon for No collision
                    _currWeapon.GetComponent<WeaponSword>().ResetVariables();

                    if (_currWeapon.GetComponent<WeaponSword>().ShouldDestroy)
                    {
                        DestroyWeapon();
                    }
                    _playerState = PlayerStates.MOVEMENT;
                }
                break;


            case AttackIndexes.RIFLE: //If we're shooting a Blunderbuss
                if (stateInfo.IsName("RifleGrip") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && !_currentlyAttacking) //If Blunderbuss Recoil animation started
                {
                    _currentlyAttacking = true;
                    playerRigidbody.AddForce(-transform.forward * _currWeapon.GetComponent<WeaponBlunderbuss>().SelfKnockbackForceMultiplier, ForceMode.Impulse);
                }

                if (stateInfo.IsName("RifleFire") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && _currentlyAttacking)//if Blunderbuss Recoil animation ended
                {
                    _currentlyAttacking = false;

                    if (_currWeapon.GetComponent<WeaponBlunderbuss>().ShouldDestroy)
                    {
                        _rightHandTransform.gameObject.GetComponent<Rigidbody>().freezeRotation = false;
                        _playerAnimationBehavior.ToggleRifleGripAnimation(false);
                        DestroyWeapon();
                    }
                    else
                    {
                        int randNr = Random.Range(0, blunderbussReload.Length);
                        FMODUnity.RuntimeManager.PlayOneShot(blunderbussReload[randNr]);
                    }
                    _playerState = PlayerStates.MOVEMENT;
                }
                break;

            case AttackIndexes.CROSSBOW: //If we're shooting a Crossbow
                if (stateInfo.IsName("RifleGrip") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && !_currentlyAttacking) //If Crossbow Recoil animation started
                {
                    _currentlyAttacking = true;
                    playerRigidbody.AddForce(-transform.forward * _currWeapon.GetComponent<WeaponCrossbow>().SelfKnockbackForceMultiplier, ForceMode.Impulse);
                }

                if (stateInfo.IsName("RifleFire") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && _currentlyAttacking)//if Crossbow Recoil animation ended
                {
                    _currWeapon.GetComponent<WeaponCrossbow>().ResetMeshDefault();
                    _currentlyAttacking = false;

                    if (_currWeapon.GetComponent<WeaponCrossbow>().ShouldDestroy)
                    {
                        _rightHandTransform.gameObject.GetComponent<Rigidbody>().freezeRotation = false;
                        _playerAnimationBehavior.ToggleRifleGripAnimation(false);
                        DestroyWeapon();
                    }
                    else
                    {
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bow/Bow string tension");
                    }

                    _playerState = PlayerStates.MOVEMENT;
                }
                break;

            case AttackIndexes.MACEFISH: //If we're attacking with a MaceFish
            case AttackIndexes.MALLET: //If we're attacking with a Mallet
                if (stateInfo.IsName("SmashWindup") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && !_currentlyAttacking) //If Attack animation started
                {
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Sword/Sword Swing Low 3");

                    _currentlyAttacking = true;
                    _currWeapon.layer = _currWeapon.GetComponent<WeaponMallet>().AttackWeaponLayer; //Change layer on weapon so we can hit players/physical objects
                }

                if (stateInfo.IsName("SmashAttack") && _playerAnimationBehavior.animationAnimator.IsInTransition(2) && _currentlyAttacking)//if Attack animation ended
                {
                    _currentlyAttacking = false;
                    _currWeapon.layer = _currWeapon.GetComponent<WeaponMallet>().DefaultWeaponLayer; //Change layer on weapon for No collision
                    _currWeapon.GetComponent<WeaponMallet>().ResetVariables();

                    if (_currWeapon.GetComponent<WeaponMallet>().ShouldDestroy)
                    {
                        DestroyWeapon();
                    }
                    _playerState = PlayerStates.MOVEMENT;
                }
                break;

            default:
                Debug.Log("Error at HandleAttack SwitchCase");
                break;
        }

    }

    private void PickupWeapon()
    {
        WeaponDestroy.gameObject.transform.position = _nearbyWeaponScript.gameObject.transform.position;
        WeaponDestroy.Play();
        GameObject weapon = Instantiate(_nearbyWeaponScript.WeaponPrefab, _rightHandTransform);
        weapon.GetComponent<WeaponBaseClass>()._PlayerMovementBehavior = this;
        weapon.GetComponent<WeaponBaseClass>().playerID = _playerController.PlayerID;
        string attackLayer = LayerMask.LayerToName(gameObject.layer) + "Weapon";
        weapon.GetComponent<WeaponBaseClass>().AttackWeaponLayer = LayerMask.NameToLayer(attackLayer);

        _currWeapon = weapon; //Set the player's currentWeapon to this new weapon.
        _currAttack = _nearbyWeaponScript.WeaponTypeIndex; //Update player's current attack according to our new weapontype

        Debug.Log(_nearbyWeaponScript.gameObject.name + " Picked Up!" + $" which is of Type: {System.Enum.GetName(typeof(AttackIndexes), _nearbyWeaponScript.WeaponTypeIndex)}");

        _nearbyWeaponScript.DestroyPickup();
        _nearbyWeaponScript = null;


    }

    public void DestroyWeapon()
    {
        DisableCurrWeapon();

        if (_currWeapon)
        {
            WeaponDestroy.gameObject.transform.position = _currWeapon.transform.position;
            WeaponDestroy.Play();
            Destroy(_currWeapon);
            _currAttack = AttackIndexes.NONE;
        }
    }

    Vector3 CameraDirection(Vector3 rotationDirection)
    {
        var forward = mainCamera.transform.forward;
        var right = mainCamera.transform.right;

        forward.y = 0.0f;
        right.y = 0.0f;

        return forward * rotationDirection.z + right * rotationDirection.x;
    }

    public void InterruptPlayerState(PlayerStates interuptAction)
    {
        switch (_playerState)
        {
            case PlayerStates.MOVEMENT:
                DisableCurrWeapon();
                _pickupIndicator.SetActive(false);
                break;
            case PlayerStates.JUMP:
                DisableCurrWeapon();
                _pickupIndicator.SetActive(false);
                break;
            case PlayerStates.ATTACK:
                _currentlyAttacking = false;
                _currWeapon.layer = _currWeapon.GetComponent<WeaponBaseClass>().DefaultWeaponLayer;
                _currWeapon.GetComponent<WeaponBaseClass>().ResetVariables();
                DisableCurrWeapon();
                break;
            case PlayerStates.PICKUP:
                if(_nearbyWeaponScript)
                    _nearbyWeaponScript.BeingLootedByPlayerID = -1;
                _currPickupTime = 0;
                _playerAnimationBehavior.TogglePickupAnimation(false);
                _pickupIndicator.SetActive(false);
                _pickupImageFill.SetActive(false);
                break;
            case PlayerStates.GRAB:
                _playerAnimationBehavior.ToggleGrabAnimation(false);
                gripModule.GrippDisable();
                break;
            case PlayerStates.GRABBED:
                PlayerSetup.instance.PlayerList[_grabbedByPlayerID].GetComponent<PlayerMovementBehaviour>().InterruptPlayerState(PlayerStates.GRABBED);
                RestorePlayerStateAfterGrabbed();
                break;
            case PlayerStates.DEAD:
                break;
            default:
                Debug.Log("Error at CopyMotion SwitchCase");
                break;

        }

        if (interuptAction == PlayerStates.GRAB) //if grabbed by other player, go to Grabbed state
        {
            _playerAnimationBehavior.ToggleGrabbedAnimation(true);
            _playerState = PlayerStates.GRABBED;
        }
        else if (interuptAction == PlayerStates.DEAD) //if hp under 0, go to Dead state
        {
            if (_healthSystem.Electric.isPlaying)
            {
                _playerAnimationBehavior.ToggleElectrocutionAnimation(true);
            }

            _playerState = PlayerStates.DEAD;
        }
        else if (interuptAction == PlayerStates.GRABBED) //if player broke free from being grabbed
        {
            _playerState = PlayerStates.MOVEMENT;
        }
    }

    public void RestorePlayerStateAfterGrabbed()
    {
        _playerAnimationBehavior.ToggleGrabbedAnimation(false);
        _grabbedByPlayerID = -1;
        _currGrabbedButtonCounter = 0;

        RestoreCurrWeapon();

        _playerState = PlayerStates.MOVEMENT;
    }

    private void HandleGrabbed()
    {
        if (_currGrabbedButtonCounter >= _requiredButtonPresses) //If player broke free
        {
            GameObject grabber = PlayerSetup.instance.PlayerList[_grabbedByPlayerID];

            grabber.GetComponent<PlayerMovementBehaviour>().InterruptPlayerState(PlayerStates.GRABBED);

            grabber.GetComponent<Rigidbody>().AddForce((grabber.transform.up * 300) + (-grabber.transform.forward * (_knocback / 2)), ForceMode.Impulse);
            gameObject.GetComponent<Rigidbody>().AddForce((grabber.transform.up * 300) + (grabber.transform.forward * (_playerController.HeavyBoots ? (_knocback / 1.25f) : _knocback)), ForceMode.Impulse);

            _grabbedIndicator.SetActive(false);
            _grabbedIndicatorFill.fillAmount = 0;

            RestorePlayerStateAfterGrabbed();
        }
    }

    public void ResetAllVariables() //Used to Reset between each stage
    {
        InterruptPlayerState(PlayerStates.GRABBED); // Cancels current state and enters Movement state
        DestroyWeapon();
        _healthSystem.ResetHealthVariables();

    }

    private void DisableCurrWeapon()
    {
        switch (_currAttack)
        {
            case AttackIndexes.RIFLE:
            case AttackIndexes.CROSSBOW:
                _playerAnimationBehavior.ToggleRifleGripAnimation(false);
                _rightHandTransform.gameObject.GetComponent<Rigidbody>().freezeRotation = false;
                break;
        }
    }
    private void RestoreCurrWeapon()
    {
        switch (_currAttack)
        {
            case AttackIndexes.RIFLE:
            case AttackIndexes.CROSSBOW:
                _playerAnimationBehavior.ToggleRifleGripAnimation(true);
                _rightHandTransform.gameObject.GetComponent<Rigidbody>().freezeRotation = true;
                break;
        }
    }
}
